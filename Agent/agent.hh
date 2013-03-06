#ifndef BOLD_AGENT_HH
#define BOLD_AGENT_HH

#include <LinuxDARwIn.h>
#include <LinuxCM730.h>
#include <opencv2/opencv.hpp>

#include "../Ambulator/ambulator.hh"
#include "../GameController/GameControllerReceiver.hh"
#include "../VisualCortex/visualcortex.hh"

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
          bool const& showUI,
          bool const& useJoystick,
          bool const& autoGetUpFromFallen,
          unsigned int const& gameControlUdpPort = GAMECONTROLLER_PORT);

    int run();

  private:
    const minIni& d_ini;
    Robot::LinuxCM730 d_linuxCM730;
    Robot::CM730 d_CM730;
    Robot::LinuxMotionTimer* d_motionTimer;
    std::string d_motionFile;
    DataStreamer* d_streamer;
    Camera* d_camera;
    VisualCortex* d_visualCortex;
    Ambulator d_ambulator;
    Joystick* d_joystick;
    bool d_showUI;
    bool d_autoGetUpFromFallen;
    GameControllerReceiver d_gameControlReceiver;

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

    bool init();

    void initCamera();

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

//    void controlHead(cv::Mat raw);
    void standUpIfFallen();
    void processInputCommands();
  };
}

#endif
