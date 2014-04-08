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
    typedef Eigen::Vector3d FilterState;

    Localiser();

    void update();

    AgentPosition position() const { return d_pos; }
    AgentPosition smoothedPosition() const { return d_smoothedPos; }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  private:
    typedef ParticleFilter<3, 50> ParticleFilterUsed;

    void predict();
    void updateSmoothedPos();
    void updateStateObject();

    FilterState createRandomState();

    Eigen::Vector3d d_lastTranslation;
    Eigen::Quaterniond d_lastQuaternion;
    double d_preNormWeightSum;

    AgentPosition d_pos;
    AgentPosition d_smoothedPos;
    MovingAverage<Eigen::Vector4d> d_avgPos;

    Setting<bool>* d_useLines;
    Setting<int>* d_minGoalsNeeded;

    FilterType d_filterType;
    std::shared_ptr<Filter<3>> d_filter;

    std::function<double()> d_fieldXRng;
    std::function<double()> d_fieldYRng;
    std::function<double()> d_thetaRng;
    std::function<double()> d_positionErrorRng;
    std::function<double()> d_angleErrorRng;
  };
}
