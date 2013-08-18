#pragma once

#include <memory>

namespace bold
{
  typedef unsigned char uchar;

  class MotionScriptFile;
  class MotionScriptPage;
  class JointSelection;

  enum class MotionScriptRunnerState { Pending, Running, Finished };

  class MotionScriptRunner
  {
  public:
    MotionScriptRunner(std::shared_ptr<MotionScriptFile> file, std::shared_ptr<MotionScriptPage> page, int index);

    MotionScriptRunnerState getState() const { return d_state; }

    bool step(std::shared_ptr<JointSelection> selectedJoints);

    unsigned getValue(uchar jointId) const { return d_values[jointId]; }
    uchar getPGain(uchar jointId) const { return d_pGains[jointId]; }

    int getCurrentPageIndex() const { return d_playingPageIndex; }

  private:
    std::shared_ptr<MotionScriptFile> d_file;
    std::shared_ptr<MotionScriptPage> d_playingPage;
    std::shared_ptr<MotionScriptPage> d_nextPage;

    int d_playingPageIndex;
    /// Whether the next step will be the first of the action
    bool d_isFirstStepOfAction;
    int d_pageStepCount;
    bool d_isRunning;
    bool d_playingFinished;

    uchar d_pGains[21];
    ushort d_values[21];

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

    ///////////////// Static
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
    ushort d_unitTimeCount;
    ushort d_unitTimeNum;
    ushort d_pauseTime;
    ushort d_unitTimeTotalNum;
    ushort d_accelStep;
    Section d_section;
    uchar d_playRepeatCount;
    ushort d_nextPageIndex;
    MotionScriptRunnerState d_state;
  };
}
