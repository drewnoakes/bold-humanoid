#pragma once

#include <Eigen/Core>
#include <functional>
#include <memory>

#include "../AgentPosition/agentposition.hh"
#include "../Filter/ParticleFilter/particlefilter.hh"
#include "../MovingAverage/movingaverage.hh"
#include "../ParticleSamplerFactory/WheelSamplerFactory/wheelsamplerfactory.hh"
#include "../Configurable/configurable.hh"

namespace bold
{
  class FieldMap;
  class Control;

  typedef ParticleFilter<3>::Particle Particle;

  class Localiser : public Configurable
  {
  public:
    Localiser(std::shared_ptr<FieldMap> fieldMap);

    void predict(Eigen::Affine3d motion);

    void update();

    std::vector<Control> getControls() const { return d_controls; }

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
    double d_randomizeRatio;
    /// smaller number gives faster falloff
    double d_rewardFalloff;
    bool d_useLines;
    unsigned d_minGoalsNeeded;

    std::shared_ptr<ParticleFilter<3>> d_filter;
    std::function<double()> d_fieldXRng;
    std::function<double()> d_fieldYRng;
    std::function<double()> d_thetaRng;
    std::vector<Control> d_controls;
    WheelSamplerFactory<3> d_wsf;
    std::function<double()> d_positionError;
    std::function<double()> d_angleError;
  };
}
