#include "motionscriptrunner.hh"

#include "../AgentState/agentstate.hh"
#include "../Math/math.hh"
#include "../MotionTask/motiontask.hh"
#include "../MX28Snapshot/mx28snapshot.hh"
#include "../StateObject/HardwareState/hardwarestate.hh"
#include "../ThreadUtil/threadutil.hh"
#include "../util/ccolor.hh"

#include <cassert>

using namespace bold;
using namespace std;

MotionScriptRunner::MotionScriptRunner(shared_ptr<MotionScript const> script)
: d_script(script),
  d_currentStageIndex(0),
  d_currentKeyFrameIndex(0),
  d_state(MotionScriptRunnerState::Pending)
{
  assert(script);
  assert(script->getStageCount());
}

// TODO can we avoid passing selectedJoints at each step, to ensure it doesn't change during execution?

bool MotionScriptRunner::step(shared_ptr<JointSelection> selectedJoints)
{
  assert(ThreadUtil::isMotionLoopThread());

  if (d_state == MotionScriptRunnerState::Finished)
  {
    log::error("MotionScriptRunner::step") << "already finished";
    return false;
  }

  //
  // Initialise
  //

  if (d_state == MotionScriptRunnerState::Pending)
  {
    // Special treatment for the first step of a new script

    d_state = MotionScriptRunnerState::Running;
    d_sectionStepIndex = 0;
    d_sectionStepCount = 0;
    d_keyFramePauseStepCount = 0;
    d_section = Section::PAUSE; // set to PAUSE so we transition to PRE immediately
    d_currentStageIndex = 0;
    d_currentStage = d_script->getStage(d_currentStageIndex);
    d_currentKeyFrameIndex = -1; // will be incremented to 0 immediately
    d_repeatCurrentStageCount = d_currentStage->repeatCount;

    memset(&d_mainAngles1024, 0, sizeof(d_mainAngles1024));

    auto hw = AgentState::get<HardwareState>();

    assert(hw);

    for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    {
      // copy the current pose
      d_values[jointId] = hw->getMX28State(jointId).presentPositionValue;

      // Only update selected joints
      if (!(*selectedJoints)[jointId])
        continue;

      d_keyFrameTargetAngles[jointId] = hw->getMX28State(jointId).presentPositionValue;
      d_sectionStartGoalSpeeds[jointId] = 0;
      d_keyFrameDeltaValue[jointId] = 0;
      d_goalSpeeds[jointId] = 0;
    }
  }

  //
  // Check if we have to progress to the next section
  //

  if (d_sectionStepIndex >= d_sectionStepCount)
  {
    assert(d_sectionStepIndex == d_sectionStepCount);

    if (!progressToNextSection(selectedJoints))
      return false;
  }

  //
  // Step within the current section
  //

  continueCurrentSection(selectedJoints);

//   int jj = (int)JointId::L_KNEE;
//   cout << (int)d_sectionStepIndex << ","
//        << (int)d_sectionStepCount << ","
//        << (int)d_keyFramePauseStepCount << ","
//        << (int)d_section << ","
//        << (int)d_keyFrameMotionStepCount << ","
//        << (int)d_accelStepCount << ","
//        << (int)d_repeatCurrentStageCount << ","
//        << ","
//        << (int)d_sectionStartAngles[jj] << ","
//        << (int)d_keyFrameTargetAngles[jj] << ","
//        << (int)d_keyFrameDeltaValue[jj] << ","
//        << (int)d_mainAngles1024[jj] << ","
//        << (int)d_accelAngles1024[jj] << ","
//        << (int)d_mainSpeeds1024[jj] << ","
//        << (int)d_sectionStartGoalSpeeds[jj] << ","
//        << (int)d_goalSpeeds[jj] << ","
//        << (int)d_finishSpeeds[jj] << ","
//        << (int)d_values[jj] << ","
//        << (int)d_pGains[jj] << ","
//        << endl;

  // TODO if anyone is listening, create and store a MotionScriptState object to allow nicer debugging in the UI

  return true;
}

bool MotionScriptRunner::progressToNextSection(shared_ptr<JointSelection> selectedJoints)
{
  d_sectionStepIndex = 0;

  //
  // Snapshot some values at the start of the section
  //

  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
  {
    if ((*selectedJoints)[jointId])
    {
      d_sectionStartAngles[jointId] = d_values[jointId];
      d_sectionStartGoalSpeeds[jointId] = d_goalSpeeds[jointId];
    }
  }

  //
  // Section state transitions
  //

  switch (d_section)
  {
    case Section::PRE:
    {
      // PRE -> MAIN

      d_section = Section::MAIN;
      d_sectionStepCount =  d_keyFrameMotionStepCount - (d_accelStepCount << 1);
      assert(d_sectionStepCount != 0);

      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      {
        if (!(*selectedJoints)[jointId])
          continue;

        switch (d_finishSpeeds[jointId])
        {
          case FinishSpeed::NON_ZERO:
          {
            // Determine
            d_mainAngles1024[jointId] = (d_keyFrameMotionStepCount - d_accelStepCount) == 0
              ? d_mainAngles1024[jointId] = 0
              : d_mainAngles1024[jointId] = (short)((((long)(d_keyFrameDeltaValue[jointId] - d_accelAngles1024[jointId])) * d_sectionStepCount) / (d_keyFrameMotionStepCount - d_accelStepCount));
            break;
          }
          case FinishSpeed::ZERO:
          {
            d_mainAngles1024[jointId] = d_keyFrameDeltaValue[jointId] - d_accelAngles1024[jointId] - (short)((((long)d_mainSpeeds1024[jointId] * d_accelStepCount * 12) / 5) >> 8);
            break;
          }
        }
      }
      break;
    }
    case Section::MAIN:
    {
      // MAIN -> POST

      d_section = Section::POST;
      d_sectionStepCount = d_accelStepCount;
      assert(d_sectionStepCount != 0);

      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      {
        if ((*selectedJoints)[jointId])
          d_mainAngles1024[jointId] = d_keyFrameDeltaValue[jointId] - d_mainAngles1024[jointId] - d_accelAngles1024[jointId];
      }
      break;
    }
    case Section::POST:
    {
      if (d_keyFramePauseStepCount)
      {
        // POST -> PAUSE

        d_section = Section::PAUSE;
        d_sectionStepCount = d_keyFramePauseStepCount;
        assert(d_sectionStepCount != 0);
      }
      else
      {
        // POST -> PRE

        d_section = Section::PRE;
      }
      break;
    }
    case Section::PAUSE:
    {
      // PAUSE -> PRE

      d_section = Section::PRE;
      memset(d_sectionStartGoalSpeeds, 0, sizeof(d_sectionStartGoalSpeeds));
      break;
    }
  }

  // If we're in the PRE section, then we must have just transitioned into it
  if (d_section == Section::PRE)
  {
    if (d_isPlayingFinished)
    {
      d_state = MotionScriptRunnerState::Finished;
      return false;
    }

    return startKeyFrame(selectedJoints);
  }

  return true;
}

bool MotionScriptRunner::startKeyFrame(shared_ptr<JointSelection> selectedJoints)
{
  //
  // Move to next key frame
  //

  d_currentKeyFrameIndex++;

  //
  // Progress the stage, if needed
  //

  auto keyFrameCount = d_currentStage->keyFrames.size();

  if (d_currentKeyFrameIndex == keyFrameCount)
  {
    // The current stage is complete

    // Check if it needs to be repeated
    d_repeatCurrentStageCount--;
    assert(d_repeatCurrentStageCount >= 0);

    if (d_repeatCurrentStageCount == 0)
    {
      // No repeats necessary, so move to next stage
      d_currentStageIndex++;
      if (d_currentStageIndex == d_script->getStageCount())
      {
        d_isPlayingFinished = true;
        d_state = MotionScriptRunnerState::Finished;
        return false;
      }
      d_currentStage = d_script->getStage(d_currentStageIndex);
      d_repeatCurrentStageCount = d_currentStage->repeatCount;
    }
    d_currentKeyFrameIndex = 0;
  }
  else if (d_currentKeyFrameIndex == keyFrameCount - 1)
  {
    // This is the last step of the current page
    bool isFinishing = d_repeatCurrentStageCount == 1 && d_currentStageIndex == d_script->getStageCount() - 1;

    if (isFinishing)
      d_isPlayingFinished = true;
  }

  //
  // Calculate the duration of portions of the new key frame
  //

  d_keyFramePauseStepCount = (((ushort)d_currentStage->keyFrames[d_currentKeyFrameIndex].pauseCycles) << 5) / d_currentStage->speed;
  d_keyFrameMotionStepCount = ((ushort)d_currentStage->keyFrames[d_currentKeyFrameIndex].moveCycles * (ushort)d_currentStage->speed) >> 5;

  if (d_keyFrameMotionStepCount == 0)
    d_keyFrameMotionStepCount = 1;

  ////////// Joint

  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
  {
    if (!(*selectedJoints)[jointId])
      continue;

    d_accelAngles1024[jointId] = 0;

    // Find current target angle
    ushort currentTargetAngle = d_currentStage->keyFrames[d_currentKeyFrameIndex].getValue(jointId) & MotionScript::INVALID_BIT_MASK
      ? d_keyFrameTargetAngles[jointId]
      : d_currentStage->keyFrames[d_currentKeyFrameIndex].getValue(jointId);

    // Update start, prev_target, curr_target
    d_sectionStartAngles[jointId] = d_keyFrameTargetAngles[jointId];
    ushort prevTargetAngle = d_keyFrameTargetAngles[jointId];
    d_keyFrameTargetAngles[jointId] = currentTargetAngle;

    // Find Moving offset
    d_keyFrameDeltaValue[jointId] = (int)(d_keyFrameTargetAngles[jointId] - d_sectionStartAngles[jointId]);

    // Find Next target angle
    ushort nextTargetAngle;
    if (d_currentKeyFrameIndex == d_currentStage->keyFrames.size())
    {
      if (d_isPlayingFinished)
      {
        nextTargetAngle = currentTargetAngle;
      }
      else
      {
        auto nextStage = d_repeatCurrentStageCount > 1
          ? d_currentStage
          : d_script->getStage(d_currentStageIndex + 1);

        nextTargetAngle = nextStage->keyFrames[0].getValue(jointId) & MotionScript::INVALID_BIT_MASK
            ? currentTargetAngle
            : nextStage->keyFrames[0].getValue(jointId);
      }
    }
    else
    {
      nextTargetAngle = d_currentStage->keyFrames[d_currentKeyFrameIndex + 1].getValue(jointId) & MotionScript::INVALID_BIT_MASK
        ? currentTargetAngle
        : d_currentStage->keyFrames[d_currentKeyFrameIndex + 1].getValue(jointId);
    }

    bool directionChanged = !(
      (prevTargetAngle < currentTargetAngle && currentTargetAngle < nextTargetAngle) ||
      (prevTargetAngle > currentTargetAngle && currentTargetAngle > nextTargetAngle)
    );

    // Find finish type
    d_finishSpeeds[jointId] = directionChanged || d_keyFramePauseStepCount || d_isPlayingFinished
      ? FinishSpeed::ZERO
      : FinishSpeed::NON_ZERO;
  }

  static const uchar DEFAULT_ACCELERATION = 32;
  d_accelStepCount = DEFAULT_ACCELERATION;
  if (d_keyFrameMotionStepCount <= (d_accelStepCount << 1))
  {
    if (d_keyFrameMotionStepCount == 0)
    {
      d_accelStepCount = 0;
    }
    else
    {
      d_accelStepCount = (d_keyFrameMotionStepCount - 1) >> 1;
      if (d_accelStepCount == 0)
        d_keyFrameMotionStepCount = 0;
    }
  }

  ulong totalTime256T = ((ulong)d_keyFrameMotionStepCount) << 1;// /128 * 256
  ulong preSectionTime256T = ((ulong)d_accelStepCount) << 1;// /128 * 256
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
      long startSpeed1024_PreTime_256T = (long)d_sectionStartGoalSpeeds[jointId] * preSectionTime256T; //  *300/1024 * 1024/720 * 256 * 2
      long movingAngle_Speed1024Scale_256T_2T = (((long)d_keyFrameDeltaValue[jointId]) * 2560L) / 12;

      d_mainSpeeds1024[jointId] = d_finishSpeeds[jointId] == FinishSpeed::ZERO
        ? (short)((movingAngle_Speed1024Scale_256T_2T - startSpeed1024_PreTime_256T) / divider2)
        : (short)((movingAngle_Speed1024Scale_256T_2T - startSpeed1024_PreTime_256T) / divider1);

      d_mainSpeeds1024[jointId] = Math::clamp(d_mainSpeeds1024[jointId], (short)-1023, (short)1023);
    }
  }

  d_sectionStepCount = d_accelStepCount;
  assert(d_sectionStepCount != 0);

  return true;
}

void MotionScriptRunner::continueCurrentSection(shared_ptr<JointSelection> selectedJoints)
{
  d_sectionStepIndex++;

  // We don't update anything if we're in the pause section
  if (d_section == Section::PAUSE)
    return;

  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
  {
    // Only update selected joints
    if (!(*selectedJoints)[jointId])
      continue;

    // Update joint value and other working variables

    short value;

    if (d_keyFrameDeltaValue[jointId] == 0)
    {
      value = d_sectionStartAngles[jointId];
    }
    else switch (d_section)
    {
      case Section::PRE:
      {
        short speedN = (short)(((long)(d_mainSpeeds1024[jointId] - d_sectionStartGoalSpeeds[jointId]) * d_sectionStepIndex) / d_sectionStepCount);
        d_goalSpeeds[jointId] = d_sectionStartGoalSpeeds[jointId] + speedN;
        d_accelAngles1024[jointId] =  (short)((((long)(d_sectionStartGoalSpeeds[jointId] + (speedN >> 1)) * d_sectionStepIndex * 144) / 15) >> 9);

        value = d_sectionStartAngles[jointId] + d_accelAngles1024[jointId];
        break;
      }
      case Section::MAIN:
      {
        // Linear interpolation
        value = d_sectionStartAngles[jointId] + (short)(((long)(d_mainAngles1024[jointId])*d_sectionStepIndex) / d_sectionStepCount);
        d_goalSpeeds[jointId] = d_mainSpeeds1024[jointId];
        break;
      }
      case Section::POST:
      {
        if (d_sectionStepIndex == d_sectionStepCount)
        {
          // In the last step of the POST section, set the angle directly equal to the target value
          value = d_keyFrameTargetAngles[jointId];
        }
        else
        {
          if (d_finishSpeeds[jointId] == FinishSpeed::ZERO)
          {
            // Decelerate towards zero
            short speedN = (short)(((long)(0 - d_sectionStartGoalSpeeds[jointId]) * d_sectionStepIndex) / d_sectionStepCount);
            d_goalSpeeds[jointId] = d_sectionStartGoalSpeeds[jointId] + speedN;
            value = d_sectionStartAngles[jointId] + (short)((((long)(d_sectionStartGoalSpeeds[jointId] + (speedN>>1)) * d_sectionStepIndex * 144) / 15) >> 9);
          }
          else
          {
            // Linear progress towards target
            assert(d_finishSpeeds[jointId] == FinishSpeed::NON_ZERO);
            value = d_sectionStartAngles[jointId] + (short)(((long)(d_mainAngles1024[jointId]) * d_sectionStepIndex) / d_sectionStepCount);
            d_goalSpeeds[jointId] = d_mainSpeeds1024[jointId];
          }
        }
        break;
      }
      default:
      {
        log::error("MotionScriptRunner::step") << "Unexpected section: " << (int)d_section;
        throw new runtime_error("Unexpected section");
      }
    }

    if (value < 0)
      value = 0;
    if (value > 0x0FFF)
      value = 0x0FFF;

    d_values[jointId] = value;

    d_pGains[jointId] = d_currentStage->getPGain(jointId);
  }
}
