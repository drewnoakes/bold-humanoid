#pragma once

#include "../stateobject.hh"

#include <Eigen/Geometry>

namespace bold
{
  class OdometryState : public StateObject
  {
  public:
    OdometryState(Eigen::Vector3d translation)
    : d_translation(translation)
    {}

    /// Gets the cumulative translation of the torso.
    /// Users of this value can compute their own delta values.
    Eigen::Vector3d const& getTranslation() const { return d_translation; };

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    Eigen::Vector3d d_translation;
  };
}
