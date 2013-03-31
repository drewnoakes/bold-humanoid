#ifndef BOLD_AGENT_HH
#define BOLD_AGENT_HH

#include <LinuxDARwIn.h>
#include <LinuxCM730.h>
#include <opencv2/opencv.hpp>
#include <Eigen/Core>
#include <memory>

#include "../Ambulator/ambulator.hh"
#include "../GameController/GameControllerReceiver.hh"
#include "../MX28Alarm/MX28Alarm.hh"

class Joystick;

namespace bold
{
  class DataStreamer;
  class Camera;

  enum State
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
          bool const& useJoystick,
          bool const& autoGetUpFromFallen,
          bool const& recordFrames,
          unsigned int const& gameControlUdpPort = GAMECONTROLLER_PORT);

    int run();

  private:
    // Settings
    const minIni& d_ini;
    bool d_haveBody;
    std::string d_motionFile;
    bool d_isRecordingFrames;
    bool d_autoGetUpFromFallen;

    // Modules
    Robot::LinuxCM730 d_linuxCM730;
    Robot::CM730 d_CM730;
    std::shared_ptr<Robot::LinuxMotionTimer> d_motionTimer;
    std::shared_ptr<DataStreamer> d_streamer;
    std::shared_ptr<Camera> d_camera;
    Ambulator d_ambulator;
    std::shared_ptr<Joystick> d_joystick;
    GameControllerReceiver d_gameControlReceiver;
    bold::MX28Alarm d_alarmLedByJointId[Robot::JointData::NUMBER_OF_JOINTS];

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

    // Methods
    bool init();

    void initCamera();

    bool initBody();

    void think();

    void readSubBoardData();

    void stand()
    {
      d_ambulator.setMoveDir(Eigen::Vector2d(0,0));
      d_ambulator.setTurnAngle(0);
    }

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
