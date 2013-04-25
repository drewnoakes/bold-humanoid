#ifndef BOLD_AGENT_HH
#define BOLD_AGENT_HH

#include <Eigen/Core>
#include <minIni.h>
#include <memory>
#include <opencv2/opencv.hpp>

#include "../MX28Alarm/mx28alarm.hh"

class Joystick;

namespace Robot
{
  class CM730;
  class LinuxCM730;
  class LinuxMotionTimer;
}

namespace bold
{
  class Ambulator;
  class BodyState;
  class Camera;
  class CameraModel;
  class DataStreamer;
  class Debugger;
  class FieldMap;
  class GameStateReceiver;
  class Localiser;
  class OptionTree;
  class Spatialiser;
  class VisualCortex;

  enum class State
  {
    S_INIT,
    S_LOOK_FOR_BALL,
    S_APPROACH_BALL,
    S_LOOK_FOR_GOAL,
    S_START_CIRCLE_BALL,
    S_CIRCLE_BALL,
    S_START_PREKICK_LOOK,
    S_PREKICK_LOOK,
    S_KICK,
    S_GET_UP
  };

  class Agent
  {
  public:
    Agent(std::string const& U2D_dev,
          minIni const& ini,
          std::string const& motionFile,
          unsigned teamNumber,
          unsigned uniformNumber,
          bool useJoystick,
          bool autoGetUpFromFallen,
          bool useOptionTree,
          bool recordFrames);

    void run();
    void stop();

  private:
    // Settings
    bool d_haveBody;
    bool d_isRunning;
    std::string d_motionFile;
    unsigned d_teamNumber;
    unsigned d_uniformNumber;
    bool d_isRecordingFrames;
    bool d_autoGetUpFromFallen;
    bool d_useOptionTree;

    // Modules
    std::shared_ptr<Robot::LinuxCM730> d_linuxCM730;
    std::shared_ptr<Robot::CM730> d_CM730;
    std::shared_ptr<Robot::LinuxMotionTimer> d_motionTimer;

    std::shared_ptr<Ambulator> d_ambulator;
    std::shared_ptr<Camera> d_camera;
    std::shared_ptr<CameraModel> d_cameraModel;
    std::shared_ptr<DataStreamer> d_streamer;
    std::shared_ptr<Debugger> d_debugger;
    std::shared_ptr<FieldMap> d_fieldMap;
    std::shared_ptr<Joystick> d_joystick;
    std::shared_ptr<Spatialiser> d_spatialiser;
    std::shared_ptr<Localiser> d_localiser;
    std::shared_ptr<VisualCortex> d_visualCortex;
    std::shared_ptr<GameStateReceiver> d_gameStateReceiver;

    // State
    /** Number of consecutive cycles during which the ball has been seen. */
    int d_ballSeenCnt;
    /** Number of consecutive cycles during which both goal posts have been seen. */
    int d_goalSeenCnt;

    double d_circleBallX;
    double d_circleBallY;
    double d_circleBallTurn;

    State d_state;

    double d_joystickXAmpMax;
    double d_joystickYAmpMax;
    double d_joystickAAmpMax;

    // Control
    std::unique_ptr<OptionTree> d_optionTree;

    // Methods
    void initCamera(minIni const& ini);

    bool initMotionManager(minIni const& ini);

    void registerStateTypes();

    void think();

    void readSubBoardData();

    void lookForBall();
    void approachBall();
    void lookForGoal();
    /** Turn in a circle, following the pan of the head.
     * If the head is facing forwards, this method should have no effect.
     */
    void circleBall();
    void preKickLook();
    void kick() {}
    void getUp() {}
    void lookAt(Eigen::Vector2f const& pos);
    void lookAtBall();
    void lookAtGoal();
    void standUpIfFallen();
    void processInputCommands();
  };
}

#endif
