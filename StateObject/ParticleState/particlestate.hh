#pragma once

#include "../StateObject/stateobject.hh"
#include "../../Filter/ParticleFilter/particlefilter.hh"

#include <memory>
#include <vector>

namespace bold
{
  /** Holds a set of ParticleFilter particles at a given point in time.
   */
  class ParticleState : public StateObject
  {
  public:
    ParticleState(std::shared_ptr<std::vector<ParticleFilter<3>::Particle> const> particles)
    : d_particles(particles)
    {}

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    std::shared_ptr<std::vector<ParticleFilter<3>::Particle> const> d_particles;
  };
}
