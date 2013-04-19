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

      writer.Double(particle.first.x());
      writer.Double(particle.first.y());
      writer.Double(particle.first.z());

      writer.EndArray();
    }
  }
  writer.EndArray();
}
