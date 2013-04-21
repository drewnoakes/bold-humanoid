#ifndef BOLD_FILTER_HH
#define BOLD_FILTER_HH

#include <Eigen/Core>
#include <functional>
#include <utility>

namespace bold
{
  /**
   * Abstract base class for filtering implementations.
   */
  template<int DIM>
  class Filter
  {
  public:
    typedef Eigen::Matrix<double,DIM,1> State;
    typedef std::pair<State,double> Particle;

    typedef std::function<State()> StateSampler;
    typedef std::function<Particle()> ParticleSampler;
    typedef std::function<ParticleSampler(std::vector<Particle> const&)> ParticleSamplerFactory;

    virtual void predict(std::function<State(State const&)> motionModel) = 0;
    virtual void update(std::function<double(State const&)> observationModel) = 0;
    virtual Particle extract() const = 0;
  };
}

#endif
