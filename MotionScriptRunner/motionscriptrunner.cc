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
  d_playingPage(page),
  d_playingPageIndex(index),
  d_isFirstStepOfAction(true),
  d_isRunning(false),
  d_state(MotionScriptRunnerState::Pending)
{
  assert(file);
  assert(page);
}

bool MotionScriptRunner::step(shared_ptr<JointSelection> selectedJoints)
{
  assert(ThreadId::isMotionLoopThread());

  if (d_state == MotionScriptRunnerState::Finished)
  {
    cerr << "[MotionScriptRunner::step] already finished -- returning false" << endl;
    return false;
  }

  auto hw = AgentState::get<HardwareState>();

  //
  // Initialise
  //

  if (d_isFirstStepOfAction)
  {
    // Special treatment for the first step of a new action

    // copy the current pose
    auto hw = AgentState::get<HardwareState>();
    assert(hw);
    for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      d_values[jointId] = hw->getMX28State(jointId)->presentPositionValue;

    d_isRunning = true;
    d_isFirstStepOfAction = false;
    d_playingFinished = false;
    d_isStopRequested = false;
    d_state = MotionScriptRunnerState::Running;
    unitTimeCount = 0;
    unitTimeNum = 0;
    pauseTime = 0;
    section = Section::PAUSE;
    d_pageStepCount = 0;
    playRepeatCount = d_playingPage->getRepeatCount();
    nextPlayPage = 0;

    for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    {
        // Only update selected joints
      if (!(*selectedJoints)[jointId])
        continue;

      targetAngles1024[jointId] = hw->getMX28State(jointId)->presentPositionValue;
      lastOutSpeeds1024[jointId] = 0;
      movingAngles1024[jointId] = 0;
      goalSpeeds1024[jointId] = 0;
    }
  }

  if (unitTimeCount < unitTimeNum)
  {
    //
    // Continue current section
    //

    unitTimeCount++;
    if (section != Section::PAUSE)
    {
      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      {
        // Only update selected joints
        if (!(*selectedJoints)[jointId])
          continue;

        if (movingAngles1024[jointId] == 0)
        {
          d_values[jointId] = startAngles1024[jointId];
        }
        else switch (section)
        {
          case Section::PRE:
          {
            short iSpeedN = (short)(((long)(mainSpeeds1024[jointId] - lastOutSpeeds1024[jointId]) * unitTimeCount) / unitTimeNum);
            goalSpeeds1024[jointId] = lastOutSpeeds1024[jointId] + iSpeedN;
            accelAngles1024[jointId] =  (short)((((long)(lastOutSpeeds1024[jointId] + (iSpeedN >> 1)) * unitTimeCount * 144) / 15) >> 9);

            d_values[jointId] = startAngles1024[jointId] + accelAngles1024[jointId];
            break;
          }
          case Section::MAIN:
          {
            d_values[jointId] = startAngles1024[jointId] + (short)(((long)(mainAngles1024[jointId])*unitTimeCount) / unitTimeNum);
            goalSpeeds1024[jointId] = mainSpeeds1024[jointId];
            break;
          }
          case Section::POST:
          {
            if (unitTimeCount == (unitTimeNum-1))
            {
              d_values[jointId] = targetAngles1024[jointId];
            }
            else
            {
              if (finishTypes[jointId] == FinishLevel::ZERO)
              {
                short iSpeedN = (short)(((long)(0 - lastOutSpeeds1024[jointId]) * unitTimeCount) / unitTimeNum);
                goalSpeeds1024[jointId] = lastOutSpeeds1024[jointId] + iSpeedN;
                d_values[jointId] = startAngles1024[jointId] + (short)((((long)(lastOutSpeeds1024[jointId] + (iSpeedN>>1)) * unitTimeCount * 144) / 15) >> 9);
              }
              else // FinishLevel::NON_ZERO
              {
                // MAIN Section
                d_values[jointId] = startAngles1024[jointId] + (short)(((long)(mainAngles1024[jointId]) * unitTimeCount) / unitTimeNum);
                goalSpeeds1024[jointId] = mainSpeeds1024[jointId];
              }
            }
            break;
          }
          default:
          {
            cerr << "[MotionScriptRunner::step] Unexpected section: " << (int)section << endl;
            throw new std::runtime_error("Unexpected section");
          }
        }

        d_pGains[jointId] = d_playingPage->getPGain(jointId);
      }
    }
  }
  else
  {
    // Completed previous section

    unitTimeCount = 0;

    for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    {
      if ((*selectedJoints)[jointId])
      {
        startAngles1024[jointId] = d_values[jointId];
        lastOutSpeeds1024[jointId] = goalSpeeds1024[jointId];
      }
    }

    // Section (PRE -> MAIN -> POST -> (PAUSE or PRE) ...)
    switch (section)
    {
      case Section::PRE:
      {
        section = Section::MAIN;
        unitTimeNum =  unitTimeTotalNum - (accelStep << 1);

        for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
        {
          if (!(*selectedJoints)[jointId])
            continue;

          switch (finishTypes[jointId])
          {
            case FinishLevel::NON_ZERO:
            {
              mainAngles1024[jointId] = (unitTimeTotalNum - accelStep) == 0
                ? mainAngles1024[jointId] = 0
                : mainAngles1024[jointId] = (short)((((long)(movingAngles1024[jointId] - accelAngles1024[jointId])) * unitTimeNum) / (unitTimeTotalNum - accelStep));
            }
            case FinishLevel::ZERO:
            {
              mainAngles1024[jointId] = movingAngles1024[jointId] - accelAngles1024[jointId] - (short)((((long)mainSpeeds1024[jointId] * accelStep * 12) / 5) >> 8);
            }
          }
        }
        break;
      }
      case Section::MAIN:
      {
        section = Section::POST;
        unitTimeNum = accelStep;

        for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
        {
          if ((*selectedJoints)[jointId])
            mainAngles1024[jointId] = movingAngles1024[jointId] - mainAngles1024[jointId] - accelAngles1024[jointId];
        }
        break;
      }
      case Section::POST:
      {
        if (pauseTime)
        {
          section = Section::PAUSE;
          unitTimeNum = pauseTime;
        }
        else
        {
          section = Section::PRE;
        }
        break;
      }
      case Section::PAUSE:
      {
        section = Section::PRE;
        memset(lastOutSpeeds1024, 0, sizeof(lastOutSpeeds1024));
        break;
      }
    }

    // PRE Section
    if (section == Section::PRE)
    {
      if (d_playingFinished)
      {
        d_isRunning = false;
        d_state = MotionScriptRunnerState::Finished;
        return false;
      }

      d_pageStepCount++;

      if (d_pageStepCount > d_playingPage->getStepCount())
      {
        d_playingPage = m_NextPlayPage;
        if (d_playingPageIndex != nextPlayPage)
          playRepeatCount = d_playingPage->getRepeatCount();
        d_pageStepCount = 1;
        d_playingPageIndex = nextPlayPage;
      }

      if (d_pageStepCount == d_playingPage->getStepCount())
      {
        if (d_isStopRequested)
        {
          nextPlayPage = d_playingPage->getExit();
        }
        else
        {
          playRepeatCount--;
          nextPlayPage = playRepeatCount > 0
            ? d_playingPageIndex
            : d_playingPage->getNext();
        }

        if (nextPlayPage == 0)
        {
          d_playingFinished = true;
        }
        else
        {
          if (d_playingPageIndex != nextPlayPage)
            m_NextPlayPage = d_file->getPageByIndex(nextPlayPage);
          else
            m_NextPlayPage = d_playingPage;

          if (m_NextPlayPage->getRepeatCount() == 0 || m_NextPlayPage->getStepCount() == 0)
            d_playingFinished = true;
        }
      }

      //////// Step
      pauseTime = (((ushort)d_playingPage->getStepPause(d_pageStepCount-1)) << 5) / d_playingPage->getSpeed();
      ushort wMaxSpeed256 = ((ushort)d_playingPage->getStepTime(d_pageStepCount-1) * (ushort)d_playingPage->getSpeed()) >> 5;
      if (wMaxSpeed256 == 0)
        wMaxSpeed256 = 1;
      ushort wMaxAngle1024 = 0;

      ////////// Joint
      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      {
        if (!(*selectedJoints)[jointId])
          continue;

        accelAngles1024[jointId] = 0;

        // Find current target angle
        ushort wCurrentTargetAngle = d_playingPage->getStepPosition(d_pageStepCount-1, jointId) & MotionScriptPage::INVALID_BIT_MASK
          ? targetAngles1024[jointId]
          : d_playingPage->getStepPosition(d_pageStepCount-1, jointId);

        // Update start, prev_target, curr_target
        startAngles1024[jointId] = targetAngles1024[jointId];
        ushort wPrevTargetAngle = targetAngles1024[jointId];
        targetAngles1024[jointId] = wCurrentTargetAngle;

        // Find Moving offset
        movingAngles1024[jointId] = (int)(targetAngles1024[jointId] - startAngles1024[jointId]);

        // Find Next target angle
        ushort wNextTargetAngle;
        if (d_pageStepCount == d_playingPage->getStepCount())
        {
          wNextTargetAngle = d_playingFinished
            ? wCurrentTargetAngle
            : m_NextPlayPage->getStepPosition(0, jointId) & MotionScriptPage::INVALID_BIT_MASK
              ? wCurrentTargetAngle
              : m_NextPlayPage->getStepPosition(0, jointId);
        }
        else
        {
          wNextTargetAngle = d_playingPage->getStepPosition(d_pageStepCount, jointId) & MotionScriptPage::INVALID_BIT_MASK
            ? wCurrentTargetAngle
            : d_playingPage->getStepPosition(d_pageStepCount, jointId);
        }

        // Find direction change
        bool bDirectionChanged;
        if (((wPrevTargetAngle < wCurrentTargetAngle) && (wCurrentTargetAngle < wNextTargetAngle))
         || ((wPrevTargetAngle > wCurrentTargetAngle) && (wCurrentTargetAngle > wNextTargetAngle)))
        {
          bDirectionChanged = false;
        }
        else
        {
          bDirectionChanged = true;
        }

        // Find finish type
        finishTypes[jointId] = bDirectionChanged || pauseTime || d_playingFinished
          ? FinishLevel::ZERO
          : FinishLevel::NON_ZERO;

        if (d_playingPage->getSchedule() == MotionScriptPageSchedule::SPEED_BASE)
        {
          // MaxAngle1024 update
          ushort wTmp = movingAngles1024[jointId] < 0
            ? -movingAngles1024[jointId]
            :  movingAngles1024[jointId];

          if (wTmp > wMaxAngle1024)
            wMaxAngle1024 = wTmp;
        }
      }

      //wUnitTimeNum = ((wMaxAngle1024*300/1024) /(wMaxSpeed256 * 720/256)) /7.8msec;
      //             = ((128*wMaxAngle1024*300/1024) /(wMaxSpeed256 * 720/256)) ;    (/7.8msec == *128)
      //             = (wMaxAngle1024*40) /(wMaxSpeed256 *3);
      unitTimeTotalNum = d_playingPage->getSchedule() == MotionScriptPageSchedule::TIME_BASE
        ? wMaxSpeed256 //TIME BASE 051025
        : (wMaxAngle1024 * 40) / (wMaxSpeed256 * 3);

      accelStep = d_playingPage->getAcceleration();
      if (unitTimeTotalNum <= (accelStep << 1))
      {
        if (unitTimeTotalNum == 0)
        {
          accelStep = 0;
        }
        else
        {
          accelStep = (unitTimeTotalNum - 1) >> 1;
          if (accelStep == 0)
            unitTimeTotalNum = 0;
        }
      }

      ulong ulTotalTime256T = ((ulong)unitTimeTotalNum) << 1;// /128 * 256
      ulong ulPreSectionTime256T = ((ulong)accelStep) << 1;// /128 * 256
      ulong ulMainTime256T = ulTotalTime256T - ulPreSectionTime256T;
      long lDivider1 = ulPreSectionTime256T + (ulMainTime256T << 1);
      long lDivider2 = (ulMainTime256T << 1);

      if (lDivider1 == 0)
        lDivider1 = 1;

      if (lDivider2 == 0)
        lDivider2 = 1;

      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      {
        if ((*selectedJoints)[jointId])
        {
          long lStartSpeed1024_PreTime_256T = (long)lastOutSpeeds1024[jointId] * ulPreSectionTime256T; //  *300/1024 * 1024/720 * 256 * 2
          long lMovingAngle_Speed1024Scale_256T_2T = (((long)movingAngles1024[jointId]) * 2560L) / 12;

          mainSpeeds1024[jointId] = finishTypes[jointId] == FinishLevel::ZERO
            ? (short)((lMovingAngle_Speed1024Scale_256T_2T - lStartSpeed1024_PreTime_256T) / lDivider2)
            : (short)((lMovingAngle_Speed1024Scale_256T_2T - lStartSpeed1024_PreTime_256T) / lDivider1);

          mainSpeeds1024[jointId] = Math::clamp(mainSpeeds1024[jointId], (short)-1023, (short)1023);
        }
      }

      unitTimeNum = accelStep;
    } // PreSection
  }

  return true;
}
