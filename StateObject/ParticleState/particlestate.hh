#pragma once

#include "../stateobject.hh"

#include <memory>
#include <vector>
#include <Eigen/Core>

namespace bold
{
  /** Holds a set of ParticleFilter particles at a given point in time.
   */
  class ParticleState : public StateObject
  {
  public:
    ParticleState(Eigen::MatrixXd const& particles, double preNormWeightSum)
      : d_particles(particles),
        d_preNormWeightSum(preNormWeightSum)
    {}

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    Eigen::MatrixXd d_particles;
    double d_preNormWeightSum;
  };
}
