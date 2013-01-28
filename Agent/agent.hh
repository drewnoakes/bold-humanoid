#ifndef BOLD_AGENT_HH
#define BOLD_AGENT_HH

#include <LinuxDARwIn.h>
#include <LinuxCM730.h>
#include <opencv.hpp>

#include <BlobDetector/blobdetector.hh>
#include "../Ambulator/ambulator.hh"
#include "../Debugger/debugger.hh"
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
    S_CIRCLE_BALL,
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
          bool const& autoGetUpFromFallen);

    int run();

  private:
    Robot::LinuxCM730 d_linuxCM730;
    Robot::CM730 d_CM730;
    Robot::LinuxMotionTimer* d_motionTimer;
    minIni d_ini;
    std::string d_motionFile;
    cv::VideoCapture d_camera;
    Debugger d_debugger;
    unsigned char* d_LUT;
    BlobDetector d_blobDetector;
    Ambulator d_ambulator;
    int d_minBallArea;
    Joystick* d_joystick;
    bool d_showUI;
    bool d_autoGetUpFromFallen;

    std::vector<Observation> d_observations;

    State d_state;

    int d_ballSeenCnt;

    double d_joystickXAmpMax;
    double d_joystickYAmpMax;
    double d_joystickAAmpMax;

    bool init();

    void think();

    std::vector<Observation> processImage(cv::Mat& image);

    std::vector<Observation>::iterator getBallObservation()
    {
      return find_if(d_observations.begin(), d_observations.end(),
		     [](Observation const& obs) { return obs.type == O_BALL; });
    }

    bool seeBall() { 
	return getBallObservation() != d_observations.end();
    }

    void lookForBall();
    void approachBall();
    void lookForGoal() {}

    /** Turn in a circle, following the pan of the head.
     * If the head is facing forwards, this method should have no effect.
     */
    void circleBall();

    void kick() {}
    void getUp() {}

    void lookAtBall();

    void controlHead(cv::Mat raw, std::vector<Observation> observations);
    void standUpIfFallen();
    void processInputCommands();
  };
}

#endif
