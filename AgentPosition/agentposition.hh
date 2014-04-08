#pragma once

#include <limits>

#include <Eigen/Core>
#include <Eigen/Geometry>

namespace bold
{
  class AgentPosition
  {
  public:
    AgentPosition()
    : d_x(std::numeric_limits<double>::signaling_NaN()),
      d_y(std::numeric_limits<double>::signaling_NaN()),
      d_theta(std::numeric_limits<double>::signaling_NaN())
    {}

    AgentPosition(double x, double y, double theta)
      : d_x(x),
        d_y(y),
        d_theta(theta)
    {}

    AgentPosition(Eigen::Vector3d const& vec)
      : d_x(vec.x()),
        d_y(vec.y()),
        d_theta(vec(2))
    {}

    AgentPosition(Eigen::Vector4d const& vec)
      : d_x(vec.x()),
        d_y(vec.y()),
        d_theta(atan2(-vec(2), vec(3)))
    {}

    double x() const { return d_x; }
    double y() const { return d_y; }
    double theta() const { return d_theta; }

    Eigen::Affine3d worldAgentTransform() const;
    Eigen::Affine3d agentWorldTransform() const;

    Eigen::Vector2d pos2d() const { return Eigen::Vector2d(d_x, d_y); }
    Eigen::Vector3d pos3d(double z = 0) const { return Eigen::Vector3d(d_x, d_y, z); }

    AgentPosition withX(double x) const { return AgentPosition(x, d_y, d_theta); }
    AgentPosition withY(double y) const { return AgentPosition(d_x, y, d_theta); }
    AgentPosition withTheta(double theta) const { return AgentPosition(d_x, d_y, theta); }

  private:
    double d_x;
    double d_y;
    double d_theta;
  };
}
