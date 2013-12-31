#pragma once

#include <Eigen/Core>
#include <functional>
#include <memory>

#include "../AgentPosition/agentposition.hh"
#include "../Filter/ParticleFilter/particlefilter.hh"
#include "../ParticleSamplerFactory/WheelSamplerFactory/wheelsamplerfactory.hh"
#include "../stats/movingaverage.hh"

namespace bold
{
  class FieldMap;
  template<typename> class Setting;

  typedef ParticleFilter<3>::Particle Particle;

  class Localiser
  {
  public:
    Localiser(std::shared_ptr<FieldMap> fieldMap);

    void predict(Eigen::Affine3d motion);

    void update();

    AgentPosition position() const { return d_pos; }
    AgentPosition smoothedPosition() const { return d_smoothedPos; }

  private:
    void updateSmoothedPos();
    void updateStateObject();

    ParticleFilter<3>::State createRandomState();

    ParticleFilter<3>::State extract(std::shared_ptr<std::vector<Particle>> const& particles);

    std::shared_ptr<std::vector<Particle>> resample(std::shared_ptr<std::vector<Particle>> const& particles, unsigned particleCount);

    AgentPosition d_pos;
    AgentPosition d_smoothedPos;
    MovingAverage<Eigen::Vector4d> d_avgPos;
    std::shared_ptr<FieldMap> d_fieldMap;

    Setting<double>* d_randomizeRatio;
    /// smaller number gives faster falloff
    Setting<double>* d_rewardFalloff;
    Setting<bool>* d_useLines;
    Setting<int>* d_minGoalsNeeded;

    std::shared_ptr<ParticleFilter<3>> d_filter;
    std::function<double()> d_fieldXRng;
    std::function<double()> d_fieldYRng;
    std::function<double()> d_thetaRng;
    WheelSamplerFactory<3> d_wsf;
    std::function<double()> d_positionErrorRng;
    std::function<double()> d_angleErrorRng;
  };
}
