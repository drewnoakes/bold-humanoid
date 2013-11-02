#pragma once

#include "../MotionScript/motionscript.hh"

#include <memory>

namespace bold
{
  // NOTE
  //
  // Motion scripts are divided across several levels:
  //
  //   Stages -> Key Frames -> Sections -> Steps

  typedef unsigned char uchar;

  class JointSelection;

  enum class MotionScriptRunnerState { Pending, Running, Finished };

  class MotionScriptRunner
  {
  public:
    MotionScriptRunner(std::shared_ptr<MotionScript const> script);

    std::string getScriptName() const { return d_script->getName(); }

    MotionScriptRunnerState getState() const { return d_state; }

    bool step(std::shared_ptr<JointSelection> selectedJoints);

    unsigned getValue(uchar jointId) const { return d_values[jointId]; }
    uchar getPGain(uchar jointId) const { return d_pGains[jointId]; }

    int getCurrentStageIndex() const { return d_currentStageIndex; }
    int getCurrentStepIndex() const { return d_currentStepIndex; }

  private:
    bool progressToNextSection(std::shared_ptr<JointSelection> selectedJoints);

    /**************************************
    * Section             /----\
    *                    /|    |\
    *        /+---------/ |    | \
    *       / |        |  |    |  \
    * -----/  |        |  |    |   \----
    *      PRE  MAIN   PRE MAIN POST PAUSE
    ***************************************/
    enum class Section : uchar { PRE = 1, MAIN = 2, POST = 3, PAUSE = 4 };
    enum class FinishLevel : uchar { ZERO = 1, NON_ZERO = 2 };

    std::shared_ptr<MotionScript const> d_script;

    std::shared_ptr<MotionScript::Stage const> d_currentStage;

    int d_currentStageIndex;
    int d_currentStepIndex;
    int d_repeatCurrentStageCount;

    // TODO can this be replaced by d_state in Finished? or new Finishing value?
    bool d_playingFinished;

    uchar d_pGains[21];
    ushort d_values[21];

    static const int JOINT_ARRAY_LENGTH = 22;

    ushort d_startAngles1024[JOINT_ARRAY_LENGTH];
    ushort d_targetAngles1024[JOINT_ARRAY_LENGTH];

    short d_movingAngles1024[JOINT_ARRAY_LENGTH];
    short d_mainAngles1024[JOINT_ARRAY_LENGTH];
    short d_accelAngles1024[JOINT_ARRAY_LENGTH];

    short d_mainSpeeds1024[JOINT_ARRAY_LENGTH];
    short d_lastOutSpeeds1024[JOINT_ARRAY_LENGTH];
    short d_goalSpeeds1024[JOINT_ARRAY_LENGTH];

    FinishLevel d_finishTypes[JOINT_ARRAY_LENGTH];

    // TODO is a 'unit' the same as a 'section'? If so, rename for clarity.
    // TODO what's the difference between d_unitTimeCount and d_unitTimeNum? Rename so it's clearer.
    ushort d_unitTimeCount;
    ushort d_unitTimeNum;
    ushort d_pauseTime;
    ushort d_unitTimeTotalNum;
    ushort d_accelStep;
    Section d_section;
    MotionScriptRunnerState d_state;
  };
}
