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
    std::shared_ptr<MotionScriptPage> m_NextPlayPage;

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
    ushort startAngles1024[JOINT_ARRAY_LENGTH];
    ushort targetAngles1024[JOINT_ARRAY_LENGTH];
    short movingAngles1024[JOINT_ARRAY_LENGTH];
    short mainAngles1024[JOINT_ARRAY_LENGTH];
    short accelAngles1024[JOINT_ARRAY_LENGTH];
    short mainSpeeds1024[JOINT_ARRAY_LENGTH];
    short lastOutSpeeds1024[JOINT_ARRAY_LENGTH];
    short goalSpeeds1024[JOINT_ARRAY_LENGTH];
    FinishLevel finishTypes[JOINT_ARRAY_LENGTH];
    ushort unitTimeCount;
    ushort unitTimeNum;
    ushort pauseTime;
    ushort unitTimeTotalNum;
    ushort accelStep;
    Section section;
    uchar playRepeatCount;
    ushort nextPlayPage;
    MotionScriptRunnerState d_state;
  };
}
