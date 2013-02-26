#ifndef BOLD_WORLD_MODEL_HH
#define BOLD_WORLD_MODEL_HH

//#include <Eigen/Core>
//#include <sigc++/sigc++.h>
#include <vector>

#include <LinuxDARwIn.h>

#include "../Agent/agent.hh"
#include "../GameState/gamestate.hh"
#include "../vision/ImageLabeller/imagelabeller.hh"
#include "../vision/LUTBuilder/lutbuilder.hh"

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

  class WorldModel
  {
  private:
    WorldModel()
    : d_minBallArea(8*8)
    {};

    int d_minBallArea;

    WorldModel(WorldModel const&);
    void operator=(WorldModel const&);

    ImageLabeller* d_imageLabeller;

  public:
    void initialise(minIni const& ini)
    {
      std::cout << "[WorldModel::WorldModel] Constructing WorldModel" << std::endl;

      goalHsvRange = hsvRange::fromConfig(ini, "Goal", 40, 10, 210, 55, 190, 65);
      ballHsvRange = hsvRange::fromConfig(ini, "Ball", 10, 15, 255, 95, 190, 95);

      hsvRanges.push_back(goalHsvRange);
      hsvRanges.push_back(ballHsvRange);

      d_minBallArea = ini.geti("Vision", "MinBallArea", 8*8);

      // Build ImageLabeller
      std::cout << "[WorldModel::initialise] Building ImageLabeller" << std::endl;
      d_imageLabeller = new ImageLabeller(bold::WorldModel::getInstance().hsvRanges);
    };

    // TODO encapsulate fields

    bold::hsvRange goalHsvRange;
    bold::hsvRange ballHsvRange;

    std::vector<bold::hsvRange> hsvRanges;

    std::vector<Observation> observations;
    std::vector<Observation> goalObservations;
    Observation ballObservation;

    bool isBallVisible;

    /**
     * Integrate a camera image into the world model.
     */
    void integrateImage(cv::Mat& cameraImage);

    /**
     * Gets the singleton instance of the WorldModel.
     */
    static WorldModel& getInstance()
    {
      static WorldModel instance;
      return instance;
    }
  };
}

#endif