#pragma once

#include <Eigen/Core>
#include <vector>
#include <string>

#include "../util/Maybe.hh"
#include "../geometry/Bounds.hh"

namespace bold
{
  class MotionScript;

  class Kick
  {
  public:
    static void loadAll();
    static std::vector<std::shared_ptr<Kick const>> const& getAll() { return d_allKicks; }

    Kick(std::string id, std::string scriptPath, Bounds2d ballBounds, Eigen::Vector2d endPos);

    /**
     * Given a ball position, returns whether the ball can be kicked and
     * where it is likely to end up.
     *
     * All positions are in the agent frame.
     */
    Maybe<Eigen::Vector2d> estimateEndPos(Eigen::Vector2d const& ballPos) const;

    std::string getId() const { return d_id; }

    std::shared_ptr<MotionScript> getMotionScript() const { return d_motionScript; }

  private:
    static std::vector<std::shared_ptr<Kick const>> d_allKicks;

    std::string d_id;
    std::shared_ptr<MotionScript> d_motionScript;
    Bounds2d d_ballBounds;
    Eigen::Vector2d d_endPos;
  };
}
