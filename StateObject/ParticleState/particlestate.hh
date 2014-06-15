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
    ParticleState(Eigen::MatrixXd const& particles, double preNormWeightSum, double smoothedPreNormWeightSum, double uncertainty)
      : d_particles{particles},
      d_preNormWeightSum{preNormWeightSum},
      d_smoothedPreNormWeightSum{smoothedPreNormWeightSum},
      d_uncertainty{uncertainty}
    {}

    Eigen::MatrixXd const& getParticles() const { return d_particles; }
    double getPreNormWeightSum() const { return d_preNormWeightSum; }
    double getSmoothedPreNormWeightSum() const { return d_smoothedPreNormWeightSum; }
    double getUncertainty() const { return d_uncertainty; }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    Eigen::MatrixXd d_particles;
    double d_preNormWeightSum;
    double d_smoothedPreNormWeightSum;
    double d_uncertainty;
  };
}
