#pragma once

#include <Eigen/Core>
#include <functional>
#include <memory>

#include "../AgentPosition/agentposition.hh"
#include "../filters/Filter/filter.hh"
#include "../stats/movingaverage.hh"
#include "../stats/lowpassfilter.hh"

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
    double uncertainty() const { return d_uncertainty; }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  private:
    typedef Eigen::Vector4d FilterState;

    std::pair<FilterState, double> generateState();

    void predict();
    void updateSmoothedPos();
    void updateStateObject();

    FilterState createRandomState();

    bool d_haveLastAgentTransform;
    Eigen::Affine3d d_lastAgentTransform;
    Eigen::Quaterniond d_lastQuaternion;
    double d_preNormWeightSum;
    LowPassFilter d_preNormWeightSumFilter;

    bool d_shouldRandomise;

    AgentPosition d_pos;
    AgentPosition d_smoothedPos;
    MovingAverage<Eigen::Vector4d> d_avgPos;
    double d_uncertainty;

    Setting<bool>* d_useLines;
    Setting<int>* d_minGoalsNeeded;
    Setting<double>* d_defaultKidnapWeight;
    Setting<double>* d_penaltyKidnapWeight;
    Setting<bool>* d_enablePenaltyRandomise;
    Setting<bool>* d_enableDynamicError;

    FilterType d_filterType;
    std::shared_ptr<Filter<4>> d_filter;

    std::function<double()> d_fieldXRng;
    std::function<double()> d_fieldYRng;
    std::function<double()> d_goalAreaXRng;
    std::function<double()> d_goalAreaYRng;
    std::function<double()> d_thetaRng;
    std::function<double()> d_positionErrorRng;
    std::function<double()> d_angleErrorRng;
  };
}
