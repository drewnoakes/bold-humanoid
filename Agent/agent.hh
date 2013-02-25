#ifndef BOLD_AGENT_HH
#define BOLD_AGENT_HH

#include <LinuxDARwIn.h>
#include <LinuxCM730.h>
#include "../vision/Camera/camera.hh"
#include "../vision/PixelFilterChain/pixelfilterchain.hh"
#include <opencv2/opencv.hpp>

#include <BlobDetector/blobdetector.hh>
#include "../Ambulator/ambulator.hh"
#include "../DataStreamer/datastreamer.hh"
#include "../Debugger/debugger.hh"
#include "../GameController/GameControllerReceiver.hh"
#include "../joystick/joystick.hh"

namespace bold
{
  enum ObsType
  {
    O_BALL,
    O_GOAL_POST,
    O_LEFT_GOAL_POST,
    O_RIGHT_GOAL_POST
  };

  struct Observation
  {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    ObsType type;
    Eigen::Vector2f pos;
  };

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
          std::string const& iniFile,
          std::string const& motionFile,
          bool const& showUI,
          bool const& useJoystick,
          bool const& autoGetUpFromFallen,
          unsigned int const& gameControlUdpPort = GAMECONTROLLER_PORT);

    int run();

  private:
    Robot::LinuxCM730 d_linuxCM730;
    Robot::CM730 d_CM730;
    Robot::LinuxMotionTimer* d_motionTimer;
    minIni d_ini;
    std::string d_motionFile;
    Camera d_camera;
    Debugger d_debugger;
    DataStreamer* d_streamer;
    unsigned char* d_LUT;
    BlobDetector d_blobDetector;
    Ambulator d_ambulator;
    int d_minBallArea;
    Joystick* d_joystick;
    bool d_showUI;
    bool d_autoGetUpFromFallen;
    GameControllerReceiver d_gameControlReceiver;

    double d_circleBallX;
    double d_circleBallY;
    double d_circleBallTurn;

    PixelFilterChain d_pfChain;

    std::vector<Observation> d_observations;

    std::vector<Observation> d_goalObservations;

    State d_state;

    /** Number of consecutive cycles during which the ball has been seen. */
    int d_ballSeenCnt;
    /** Number of consecutive cycles during which both goal posts have been seen. */
    int d_goalSeenCnt;

    double d_joystickXAmpMax;
    double d_joystickYAmpMax;
    double d_joystickAAmpMax;

    bool init();

    void initCamera();

    void think();

    void readSubBoardData();

    std::vector<Observation> processImage(cv::Mat& image);

    std::vector<Observation>::iterator getBallObservation()
    {
      return find_if(d_observations.begin(), d_observations.end(),
        [](Observation const& obs) { return obs.type == O_BALL; });
    }

    bool seeBall()
    {
      return getBallObservation() != d_observations.end();
    }

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

    void controlHead(cv::Mat raw, std::vector<Observation> observations);
    void standUpIfFallen();
    void processInputCommands();
  };
}

#endif
