#ifndef BOLD_WORLD_MODEL_HH
#define BOLD_WORLD_MODEL_HH

#include <minIni.h>
#include <iostream>
#include <vector>
#include <Eigen/Core>

#include "../geometry/LineSegment.hh"

namespace bold
{
  class WorldModel
  {
  public:
    void initialise(minIni const& ini);

    std::vector<bold::LineSegment2d> getFieldLines() { return d_fieldLines; }

    /** Positions of the base of four goal posts. */
    std::vector<Eigen::Vector2d> getGoalPostPositions() { return d_goalPostPositions; }

    /** Gets the singleton instance of the WorldModel. */
    static WorldModel& getInstance()
    {
      static WorldModel instance;
      return instance;
    }

  private:
    std::vector<bold::LineSegment2d> d_fieldLines;
    std::vector<Eigen::Vector2d> d_goalPostPositions;

    WorldModel() {};

    WorldModel(WorldModel const&) = delete;
    void operator=(WorldModel const&) = delete;
  };
}

#endif
