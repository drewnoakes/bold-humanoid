#include "motionscriptrunner.hh"

#include "../AgentState/agentstate.hh"
#include "../Math/math.hh"
#include "../MotionScriptFile/motionscriptfile.hh"
#include "../MotionScriptPage/motionscriptpage.hh"
#include "../MotionTask/motiontask.hh"
#include "../MX28Snapshot/mx28snapshot.hh"
#include "../StateObject/HardwareState/hardwarestate.hh"
#include "../ThreadId/threadid.hh"

#include <cassert>

using namespace bold;
using namespace std;

MotionScriptRunner::MotionScriptRunner(shared_ptr<MotionScriptFile> file, shared_ptr<MotionScriptPage> page, int index)
: d_file(file),
  d_currentPage(page),
  d_currentPageIndex(index),
  d_state(MotionScriptRunnerState::Pending)
{
  assert(file);
  assert(page);
  assert(d_currentPage->getRepeatCount());
}

bool MotionScriptRunner::step(shared_ptr<JointSelection> selectedJoints)
{
  assert(ThreadId::isMotionLoopThread());

  if (d_state == MotionScriptRunnerState::Finished)
  {
    cerr << "[MotionScriptRunner::step] already finished" << endl;
    return false;
  }

  auto hw = AgentState::get<HardwareState>();

  //
  // Initialise
  //

  if (d_state == MotionScriptRunnerState::Pending)
  {
    // Special treatment for the first step of a new action

    // copy the current pose
    auto hw = AgentState::get<HardwareState>();
    assert(hw);
    for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      d_values[jointId] = hw->getMX28State(jointId)->presentPositionValue;

    d_state = MotionScriptRunnerState::Running;
    d_unitTimeCount = 0;
    d_unitTimeNum = 0;
    d_pauseTime = 0;
    d_section = Section::PAUSE;
    d_currentPageStep = 0;
    d_repeatCurrentPageCount = d_currentPage->getRepeatCount();
    d_nextPageIndex = 0;

    for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    {
        // Only update selected joints
      if (!(*selectedJoints)[jointId])
        continue;

      d_targetAngles1024[jointId] = hw->getMX28State(jointId)->presentPositionValue;
      d_lastOutSpeeds1024[jointId] = 0;
      d_movingAngles1024[jointId] = 0;
      d_goalSpeeds1024[jointId] = 0;
    }
  }

  if (d_unitTimeCount < d_unitTimeNum)
  {
    //
    // Continue current section
    //

    d_unitTimeCount++;
    if (d_section != Section::PAUSE)
    {
      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      {
        // Only update selected joints
        if (!(*selectedJoints)[jointId])
          continue;

        if (d_movingAngles1024[jointId] == 0)
        {
          d_values[jointId] = d_startAngles1024[jointId];
        }
        else switch (d_section)
        {
          case Section::PRE:
          {
            short iSpeedN = (short)(((long)(d_mainSpeeds1024[jointId] - d_lastOutSpeeds1024[jointId]) * d_unitTimeCount) / d_unitTimeNum);
            d_goalSpeeds1024[jointId] = d_lastOutSpeeds1024[jointId] + iSpeedN;
            d_accelAngles1024[jointId] =  (short)((((long)(d_lastOutSpeeds1024[jointId] + (iSpeedN >> 1)) * d_unitTimeCount * 144) / 15) >> 9);

            d_values[jointId] = d_startAngles1024[jointId] + d_accelAngles1024[jointId];
            break;
          }
          case Section::MAIN:
          {
            d_values[jointId] = d_startAngles1024[jointId] + (short)(((long)(d_mainAngles1024[jointId])*d_unitTimeCount) / d_unitTimeNum);
            d_goalSpeeds1024[jointId] = d_mainSpeeds1024[jointId];
            break;
          }
          case Section::POST:
          {
            if (d_unitTimeCount == (d_unitTimeNum-1))
            {
              d_values[jointId] = d_targetAngles1024[jointId];
            }
            else
            {
              if (d_finishTypes[jointId] == FinishLevel::ZERO)
              {
                short speedN = (short)(((long)(0 - d_lastOutSpeeds1024[jointId]) * d_unitTimeCount) / d_unitTimeNum);
                d_goalSpeeds1024[jointId] = d_lastOutSpeeds1024[jointId] + speedN;
                d_values[jointId] = d_startAngles1024[jointId] + (short)((((long)(d_lastOutSpeeds1024[jointId] + (speedN>>1)) * d_unitTimeCount * 144) / 15) >> 9);
              }
              else // FinishLevel::NON_ZERO
              {
                // MAIN Section
                d_values[jointId] = d_startAngles1024[jointId] + (short)(((long)(d_mainAngles1024[jointId]) * d_unitTimeCount) / d_unitTimeNum);
                d_goalSpeeds1024[jointId] = d_mainSpeeds1024[jointId];
              }
            }
            break;
          }
          default:
          {
            cerr << "[MotionScriptRunner::step] Unexpected section: " << (int)d_section << endl;
            throw new std::runtime_error("Unexpected section");
          }
        }

        d_pGains[jointId] = d_currentPage->getPGain(jointId);
      }
    }
  }
  else
  {
    // Completed previous section

    d_unitTimeCount = 0;

    for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    {
      if ((*selectedJoints)[jointId])
      {
        d_startAngles1024[jointId] = d_values[jointId];
        d_lastOutSpeeds1024[jointId] = d_goalSpeeds1024[jointId];
      }
    }

    // Section (PRE -> MAIN -> POST -> (PAUSE or PRE) ...)
    switch (d_section)
    {
      case Section::PRE:
      {
        d_section = Section::MAIN;
        d_unitTimeNum =  d_unitTimeTotalNum - (d_accelStep << 1);

        for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
        {
          if (!(*selectedJoints)[jointId])
            continue;

          switch (d_finishTypes[jointId])
          {
            case FinishLevel::NON_ZERO:
            {
              d_mainAngles1024[jointId] = (d_unitTimeTotalNum - d_accelStep) == 0
                ? d_mainAngles1024[jointId] = 0
                : d_mainAngles1024[jointId] = (short)((((long)(d_movingAngles1024[jointId] - d_accelAngles1024[jointId])) * d_unitTimeNum) / (d_unitTimeTotalNum - d_accelStep));
            }
            case FinishLevel::ZERO:
            {
              d_mainAngles1024[jointId] = d_movingAngles1024[jointId] - d_accelAngles1024[jointId] - (short)((((long)d_mainSpeeds1024[jointId] * d_accelStep * 12) / 5) >> 8);
            }
          }
        }
        break;
      }
      case Section::MAIN:
      {
        d_section = Section::POST;
        d_unitTimeNum = d_accelStep;

        for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
        {
          if ((*selectedJoints)[jointId])
            d_mainAngles1024[jointId] = d_movingAngles1024[jointId] - d_mainAngles1024[jointId] - d_accelAngles1024[jointId];
        }
        break;
      }
      case Section::POST:
      {
        if (d_pauseTime)
        {
          d_section = Section::PAUSE;
          d_unitTimeNum = d_pauseTime;
        }
        else
        {
          d_section = Section::PRE;
        }
        break;
      }
      case Section::PAUSE:
      {
        d_section = Section::PRE;
        memset(d_lastOutSpeeds1024, 0, sizeof(d_lastOutSpeeds1024));
        break;
      }
    }

    // If we're in the PRE section, then we must have just transitioned into it
    if (d_section == Section::PRE)
    {
      if (d_playingFinished)
      {
        d_state = MotionScriptRunnerState::Finished;
        return false;
      }

      // Move to next page
      d_currentPageStep++;

      if (d_currentPageStep > d_currentPage->getStepCount())
      {
        // The current page is complete -- progress to the 'next' page
        assert(d_nextPage);
        d_currentPage = d_nextPage;
        d_repeatCurrentPageCount = d_currentPage->getRepeatCount();
        d_currentPageStep = 1;
        d_currentPageIndex = d_nextPageIndex;
      }

      if (d_currentPageStep == d_currentPage->getStepCount())
      {
        // This is the last step of the current page -- prepare the 'next' page, if there is one
        d_repeatCurrentPageCount--;
        d_nextPageIndex = d_repeatCurrentPageCount > 0
          ? d_currentPageIndex
          : d_currentPage->getNext();

        if (d_nextPageIndex == 0)
        {
          d_playingFinished = true;
        }
        else
        {
          if (d_currentPageIndex != d_nextPageIndex)
            d_nextPage = d_file->getPageByIndex(d_nextPageIndex);
          else
            d_nextPage = d_currentPage;

          if (d_nextPage->getRepeatCount() == 0 || d_nextPage->getStepCount() == 0)
            d_playingFinished = true;
        }
      }

      //////// Step
      d_pauseTime = (((ushort)d_currentPage->getStepPause(d_currentPageStep-1)) << 5) / d_currentPage->getSpeed();
      ushort maxSpeed256 = ((ushort)d_currentPage->getStepTime(d_currentPageStep-1) * (ushort)d_currentPage->getSpeed()) >> 5;
      if (maxSpeed256 == 0)
        maxSpeed256 = 1;
      ushort maxAngle1024 = 0;

      ////////// Joint
      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      {
        if (!(*selectedJoints)[jointId])
          continue;

        d_accelAngles1024[jointId] = 0;

        // Find current target angle
        ushort currentTargetAngle = d_currentPage->getStepPosition(d_currentPageStep-1, jointId) & MotionScriptPage::INVALID_BIT_MASK
          ? d_targetAngles1024[jointId]
          : d_currentPage->getStepPosition(d_currentPageStep-1, jointId);

        // Update start, prev_target, curr_target
        d_startAngles1024[jointId] = d_targetAngles1024[jointId];
        ushort prevTargetAngle = d_targetAngles1024[jointId];
        d_targetAngles1024[jointId] = currentTargetAngle;

        // Find Moving offset
        d_movingAngles1024[jointId] = (int)(d_targetAngles1024[jointId] - d_startAngles1024[jointId]);

        // Find Next target angle
        ushort nextTargetAngle;
        if (d_currentPageStep == d_currentPage->getStepCount())
        {
          nextTargetAngle = d_playingFinished
            ? currentTargetAngle
            : d_nextPage->getStepPosition(0, jointId) & MotionScriptPage::INVALID_BIT_MASK
              ? currentTargetAngle
              : d_nextPage->getStepPosition(0, jointId);
        }
        else
        {
          nextTargetAngle = d_currentPage->getStepPosition(d_currentPageStep, jointId) & MotionScriptPage::INVALID_BIT_MASK
            ? currentTargetAngle
            : d_currentPage->getStepPosition(d_currentPageStep, jointId);
        }

        bool directionChanged = !(
          (prevTargetAngle < currentTargetAngle && currentTargetAngle < nextTargetAngle) ||
          (prevTargetAngle > currentTargetAngle && currentTargetAngle > nextTargetAngle)
        );

        // Find finish type
        d_finishTypes[jointId] = directionChanged || d_pauseTime || d_playingFinished
          ? FinishLevel::ZERO
          : FinishLevel::NON_ZERO;

        if (d_currentPage->getSchedule() == MotionScriptPageSchedule::SPEED_BASE)
        {
          // MaxAngle1024 update
          ushort tmp = d_movingAngles1024[jointId] < 0
            ? -d_movingAngles1024[jointId]
            :  d_movingAngles1024[jointId];

          if (tmp > maxAngle1024)
            maxAngle1024 = tmp;
        }
      }

      //wUnitTimeNum = ((wMaxAngle1024*300/1024) /(wMaxSpeed256 * 720/256)) /7.8msec;
      //             = ((128*wMaxAngle1024*300/1024) /(wMaxSpeed256 * 720/256)) ;    (/7.8msec == *128)
      //             = (wMaxAngle1024*40) /(wMaxSpeed256 *3);
      d_unitTimeTotalNum = d_currentPage->getSchedule() == MotionScriptPageSchedule::TIME_BASE
        ? maxSpeed256 //TIME BASE 051025
        : (maxAngle1024 * 40) / (maxSpeed256 * 3);

      d_accelStep = d_currentPage->getAcceleration();
      if (d_unitTimeTotalNum <= (d_accelStep << 1))
      {
        if (d_unitTimeTotalNum == 0)
        {
          d_accelStep = 0;
        }
        else
        {
          d_accelStep = (d_unitTimeTotalNum - 1) >> 1;
          if (d_accelStep == 0)
            d_unitTimeTotalNum = 0;
        }
      }

      ulong totalTime256T = ((ulong)d_unitTimeTotalNum) << 1;// /128 * 256
      ulong preSectionTime256T = ((ulong)d_accelStep) << 1;// /128 * 256
      ulong mainTime256T = totalTime256T - preSectionTime256T;
      long divider1 = preSectionTime256T + (mainTime256T << 1);
      long divider2 = (mainTime256T << 1);

      if (divider1 == 0)
        divider1 = 1;

      if (divider2 == 0)
        divider2 = 1;

      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      {
        if ((*selectedJoints)[jointId])
        {
          long startSpeed1024_PreTime_256T = (long)d_lastOutSpeeds1024[jointId] * preSectionTime256T; //  *300/1024 * 1024/720 * 256 * 2
          long movingAngle_Speed1024Scale_256T_2T = (((long)d_movingAngles1024[jointId]) * 2560L) / 12;

          d_mainSpeeds1024[jointId] = d_finishTypes[jointId] == FinishLevel::ZERO
            ? (short)((movingAngle_Speed1024Scale_256T_2T - startSpeed1024_PreTime_256T) / divider2)
            : (short)((movingAngle_Speed1024Scale_256T_2T - startSpeed1024_PreTime_256T) / divider1);

          d_mainSpeeds1024[jointId] = Math::clamp(d_mainSpeeds1024[jointId], (short)-1023, (short)1023);
        }
      }

      d_unitTimeNum = d_accelStep;
    } // PreSection
  }

  return true;
}
