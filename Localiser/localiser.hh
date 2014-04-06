#pragma once

#include <Eigen/Core>
#include <functional>
#include <memory>

#include "../AgentPosition/agentposition.hh"
#include "../filters/Filter/KalmanFilter/kalmanfilter.hh"
#include "../filters/Filter/ParticleFilter/particlefilter.hh"
#include "../stats/movingaverage.hh"

namespace bold
{
  template<typename> class Setting;

  enum class FilterType
  {
    Particle = 0,
    Kalman = 1
  };

  class Localiser
  {
  public:
    Localiser();

    void update();

    AgentPosition position() const { return d_pos; }
    AgentPosition smoothedPosition() const { return d_smoothedPos; }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  private:
    typedef Eigen::Vector4d FilterState;
    typedef ParticleFilter<4, 50> ParticleFilterUsed;

    void predict();
    void updateSmoothedPos();
    void updateStateObject();

    FilterState createRandomState();

    bool d_haveLastAgentTransform;
    Eigen::Affine3d d_lastAgentTransform;
    Eigen::Quaterniond d_lastQuaternion;
    double d_preNormWeightSum;

    AgentPosition d_pos;
    AgentPosition d_smoothedPos;
    MovingAverage<Eigen::Vector4d> d_avgPos;

    Setting<bool>* d_useLines;
    Setting<int>* d_minGoalsNeeded;
    Setting<double>* d_defaultKidnapWeight;
    Setting<double>* d_penaltyKidnapWeight;

    FilterType d_filterType;
    std::shared_ptr<Filter<4>> d_filter;

    std::function<double()> d_fieldXRng;
    std::function<double()> d_fieldYRng;
    std::function<double()> d_thetaRng;
    std::function<double()> d_positionErrorRng;
    std::function<double()> d_angleErrorRng;
  };
}
