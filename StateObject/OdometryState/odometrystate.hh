#pragma once

#include "../stateobject.hh"

#include <Eigen/Geometry>

namespace bold
{
  class OdometryState : public StateObject
  {
  public:
    OdometryState(Eigen::Affine3d transform)
      : d_transform{std::move(transform)}
    {}

    /** Gets the cumulative transform of the agent frame
     * Users of this value can compute their own delta values.
     */
    Eigen::Affine3d const& getTransform() const { return d_transform; };

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    Eigen::Affine3d d_transform;
  };
}
