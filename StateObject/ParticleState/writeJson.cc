#include "particlestate.hh"

using namespace bold;
using namespace rapidjson;

void ParticleState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartArray();
  {
    for (auto const& particle : *d_particles)
    {
      writer.StartArray();

      writer.Double(particle.first.x()); // x
      writer.Double(particle.first.y()); // y
      writer.Double(particle.first.z()); // theta
      writer.Double(particle.second); // weight

      writer.EndArray();
    }
  }
  writer.EndArray();
}
