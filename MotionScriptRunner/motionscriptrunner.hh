#pragma once

#include "../MotionScript/motionscript.hh"

#include <memory>

namespace bold
{
  //
  // Motion scripts are divided across several levels:
  //
  //     Motion Script -> Stages -> Key Frames -> Sections -> Steps
  //
  // Motion Scripts
  //
  // - have a name
  // - contain one or more stages
  //
  // Stages
  //
  // - may be repeated N times in a row
  // - can specify p-gains and a speed value
  // - contain one or more key frames
  //
  // Key Frames
  //
  // - specify a set of target positions for motors
  // - specify how many cycles it takes to reach this target
  // - specify an optional number of cycles to pause for after reaching the target
  //
  // During the playback of the script, the time between key frames is divided
  // into sections: PRE, MAIN, POST, PAUSE
  //
  // Sections
  //
  //   PRE
  //     - The initial period of movement between key frames
  //     - Speed will slowly accelerate if coming from a zero finish speed
  //     - Speed will be constant if coming from a non-zero finish speed
  //
  //   MAIN
  //     - A, sometimes short, period of time between PRE and POST
  //     - May involve a very sudden change in position, depending upon the
  //       distance to travel and moveCycles
  //
  //   POST
  //     - The final period of movement between key frames
  //     - Speed will slowly decelerate if approaching a zero finish speed
  //     - Speed will be constant if approaching a non-zero finish speed
  //
  // Steps
  //
  // - another name for 'cycle'
  // - the discrete calculation unit for motor positioning
  // - 8ms apart (125 Hz)
  //

  typedef unsigned char uchar;

  class JointSelection;

  enum class MotionScriptRunnerState { Pending, Running, Finished };

  class MotionScriptRunner
  {
  public:
    static std::string getStateName(MotionScriptRunnerState const& state)
    {
      switch (state)
      {
        case MotionScriptRunnerState::Pending: return "Pending";
        case MotionScriptRunnerState::Running: return "Running";
        case MotionScriptRunnerState::Finished: return "Finished";
        default: return "Unknown";
      }
    }

    MotionScriptRunner(std::shared_ptr<MotionScript const> script);

    std::string getScriptName() const { return d_script->getName(); }

    MotionScriptRunnerState getState() const { return d_state; }

    bool step(std::shared_ptr<JointSelection> selectedJoints);

    unsigned getValue(uchar jointId) const { return d_values[jointId]; }
    uchar getPGain(uchar jointId) const { return d_pGains[jointId]; }

    int getCurrentStageIndex() const { return d_currentStageIndex; }
    int getCurrentKeyFrameIndex() const { return d_currentKeyFrameIndex; }

  private:
    bool progressToNextSection(std::shared_ptr<JointSelection> selectedJoints);
    void continueCurrentSection(std::shared_ptr<JointSelection> selectedJoints);
    bool startKeyFrame(std::shared_ptr<JointSelection> selectedJoints);

    enum class Section : uchar { PRE = 1, MAIN = 2, POST = 3, PAUSE = 4 };
    enum class FinishSpeed : uchar { ZERO = 1, NON_ZERO = 2 };

    std::shared_ptr<MotionScript const> d_script;

    std::shared_ptr<MotionScript::Stage const> d_currentStage;

    int d_currentStageIndex;
    /// Zero-based index of the current keyframe
    int d_currentKeyFrameIndex;
    /// The number of times that the current stage must be replayed, including the current execution
    int d_repeatCurrentStageCount;

    // TODO can this be replaced by d_state in Finished? or new Finishing value?
    bool d_isPlayingFinished;

    uchar d_pGains[21];
    ushort d_values[21];

    static const int JOINT_ARRAY_LENGTH = 22;

    /// Position values as at start of current keyframe
    ushort d_sectionStartAngles[JOINT_ARRAY_LENGTH];

    /// Target position values for the end of the current keyframe
    ushort d_keyFrameTargetAngles[JOINT_ARRAY_LENGTH];

    /// The total delta in position value for the current keyframe (keyframe target - keyframe start angle)
    short d_keyFrameDeltaValue[JOINT_ARRAY_LENGTH];
    short d_mainAngles1024[JOINT_ARRAY_LENGTH];
    short d_accelAngles1024[JOINT_ARRAY_LENGTH];

    short d_mainSpeeds1024[JOINT_ARRAY_LENGTH];
    /// The goal speed value at the start of the section
    short d_sectionStartGoalSpeeds[JOINT_ARRAY_LENGTH];
    short d_goalSpeeds[JOINT_ARRAY_LENGTH];

    /// Whether a joint's speed must be zero at the end of the keyframe or not
    FinishSpeed d_finishSpeeds[JOINT_ARRAY_LENGTH];

    ushort d_sectionStepIndex;
    ushort d_sectionStepCount;

    /// The number of steps to pause motion for at the end of the keyframe (zero denotes no pause)
    ushort d_keyFramePauseStepCount;

    /// The number of steps motion occurs for within this keyframe, excluding pause steps
    ushort d_keyFrameMotionStepCount;

    /// The length of the acceleration/deceleration periods in the current keyframe
    ushort d_accelStepCount;

    /// The current section within the current keyframe
    Section d_section;

    MotionScriptRunnerState d_state;
  };
}
