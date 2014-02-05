#pragma once

#include <Eigen/Core>
#include <functional>
#include <memory>

#include "../AgentPosition/agentposition.hh"
#include "../filters/Filter/ParticleFilter/particlefilter.hh"
#include "../stats/movingaverage.hh"

namespace bold
{
  class FieldMap;
  template<typename> class Setting;

  class Localiser
  {
  public:
    Localiser(std::shared_ptr<FieldMap> fieldMap);

    void update();

    AgentPosition position() const { return d_pos; }
    AgentPosition smoothedPosition() const { return d_smoothedPos; }

  private:
    void predict();
    void updateSmoothedPos();
    void updateStateObject();

    ParticleFilter<3,50>::State createRandomState();

    Eigen::Vector3d d_lastTranslation;
    Eigen::Quaterniond d_lastQuaternion;
    double d_preNormWeightSum;

    AgentPosition d_pos;
    AgentPosition d_smoothedPos;
    MovingAverage<Eigen::Vector4d> d_avgPos;
    std::shared_ptr<FieldMap> d_fieldMap;

    Setting<double>* d_randomizeRatio;
    /// smaller number gives faster falloff
    Setting<double>* d_rewardFalloff;
    Setting<bool>* d_useLines;
    Setting<int>* d_minGoalsNeeded;

    std::shared_ptr<ParticleFilter<3,50>> d_filter;
    std::function<double()> d_fieldXRng;
    std::function<double()> d_fieldYRng;
    std::function<double()> d_thetaRng;
    std::function<double()> d_positionErrorRng;
    std::function<double()> d_angleErrorRng;
  };
}
