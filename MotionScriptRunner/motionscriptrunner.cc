#include "motionscriptrunner.hh"

#include "../AgentState/agentstate.hh"
#include "../Math/math.hh"
#include "../MotionTask/motiontask.hh"
#include "../MX28Snapshot/mx28snapshot.hh"
#include "../StateObject/HardwareState/hardwarestate.hh"
#include "../ThreadId/threadid.hh"

#include <cassert>

using namespace bold;
using namespace std;

MotionScriptRunner::MotionScriptRunner(shared_ptr<MotionScript const> script)
: d_script(script),
  d_currentStageIndex(0),
  d_currentStepIndex(0),
  d_state(MotionScriptRunnerState::Pending)
{
  assert(script);
  assert(script->getStageCount());
}

// TODO can we avoid passing selectedJoints at each step, to ensure it doesn't change during execution?

bool MotionScriptRunner::step(shared_ptr<JointSelection> selectedJoints)
{
  assert(ThreadId::isMotionLoopThread());

  if (d_state == MotionScriptRunnerState::Finished)
  {
    cerr << "[MotionScriptRunner::step] already finished" << endl;
    return false;
  }

  //
  // Initialise
  //

  if (d_state == MotionScriptRunnerState::Pending)
  {
    // Special treatment for the first step of a new action

    d_state = MotionScriptRunnerState::Running;
    d_unitTimeCount = 0;
    d_unitTimeNum = 0;
    d_pauseTime = 0;
    d_section = Section::PAUSE;
    d_currentStageIndex = 0;
    d_currentStage = d_script->getStage(d_currentStageIndex);
    d_currentStepIndex = 0;
    d_repeatCurrentStageCount = d_currentStage->repeatCount;

    auto hw = AgentState::get<HardwareState>();

    assert(hw);

    for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    {
      // copy the current pose
      d_values[jointId] = hw->getMX28State(jointId)->presentPositionValue;

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
            short speedN = (short)(((long)(d_mainSpeeds1024[jointId] - d_lastOutSpeeds1024[jointId]) * d_unitTimeCount) / d_unitTimeNum);
            d_goalSpeeds1024[jointId] = d_lastOutSpeeds1024[jointId] + speedN;
            d_accelAngles1024[jointId] =  (short)((((long)(d_lastOutSpeeds1024[jointId] + (speedN >> 1)) * d_unitTimeCount * 144) / 15) >> 9);

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

        d_pGains[jointId] = d_currentStage->getPGain(jointId);
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

      // Move to next step
      d_currentStepIndex++;

      if (d_currentStepIndex == d_currentStage->steps.size())
      {
        // The current stage is complete
        // Check if it needs to be repeated
        d_repeatCurrentStageCount--;
        assert(d_repeatCurrentStageCount >= 0);
        if (d_repeatCurrentStageCount == 0)
        {
          // No repeats necessary, so move to next stage
          d_currentStageIndex++;
          assert(d_currentStageIndex < d_script->getStageCount());
          d_currentStage = d_script->getStage(d_currentStageIndex);
          d_repeatCurrentStageCount = d_currentStage->repeatCount;
        }
        d_currentStepIndex = 0;
      }
      else if (d_currentStepIndex == d_currentStage->steps.size() - 1)
      {
        // This is the last step of the current page
        bool isFinishing = d_repeatCurrentStageCount == 1 && d_currentStageIndex == d_script->getStageCount() - 1;

        if (isFinishing)
          d_playingFinished = true;
      }

      //////// Step
      d_pauseTime = (((ushort)d_currentStage->steps[d_currentStepIndex].pauseCycles) << 5) / d_currentStage->speed;
      ushort maxSpeed256 = ((ushort)d_currentStage->steps[d_currentStepIndex].moveCycles * (ushort)d_currentStage->speed) >> 5;
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
        ushort currentTargetAngle = d_currentStage->steps[d_currentStepIndex].getValue(jointId) & MotionScript::INVALID_BIT_MASK
          ? d_targetAngles1024[jointId]
          : d_currentStage->steps[d_currentStepIndex].getValue(jointId);

        // Update start, prev_target, curr_target
        d_startAngles1024[jointId] = d_targetAngles1024[jointId];
        ushort prevTargetAngle = d_targetAngles1024[jointId];
        d_targetAngles1024[jointId] = currentTargetAngle;

        // Find Moving offset
        d_movingAngles1024[jointId] = (int)(d_targetAngles1024[jointId] - d_startAngles1024[jointId]);

        // Find Next target angle
        ushort nextTargetAngle;
        if (d_currentStepIndex == d_currentStage->steps.size())
        {
          if (d_playingFinished)
          {
            nextTargetAngle = currentTargetAngle;
          }
          else
          {
            auto nextStage = d_repeatCurrentStageCount > 1
              ? d_currentStage
              : d_script->getStage(d_currentStageIndex + 1);

            nextTargetAngle = nextStage->steps[0].getValue(jointId) & MotionScript::INVALID_BIT_MASK
                ? currentTargetAngle
                : nextStage->steps[0].getValue(jointId);
          }
        }
        else
        {
          nextTargetAngle = d_currentStage->steps[d_currentStepIndex + 1].getValue(jointId) & MotionScript::INVALID_BIT_MASK
            ? currentTargetAngle
            : d_currentStage->steps[d_currentStepIndex + 1].getValue(jointId);
        }

        bool directionChanged = !(
          (prevTargetAngle < currentTargetAngle && currentTargetAngle < nextTargetAngle) ||
          (prevTargetAngle > currentTargetAngle && currentTargetAngle > nextTargetAngle)
        );

        // Find finish type
        d_finishTypes[jointId] = directionChanged || d_pauseTime || d_playingFinished
          ? FinishLevel::ZERO
          : FinishLevel::NON_ZERO;
      }

      d_unitTimeTotalNum =  maxSpeed256;

      static const uchar DEFAULT_ACCELERATION = 32;
      d_accelStep = DEFAULT_ACCELERATION;
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
