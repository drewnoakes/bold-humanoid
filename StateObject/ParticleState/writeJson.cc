#include "particlestate.hh"

using namespace bold;
using namespace rapidjson;

void ParticleState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartArray();
  {
    for (unsigned i = 0; i < d_particles.cols(); ++i)
    {
      writer.StartArray();

      // TODO find a way to write these numbers with less precision, to keep the JSON size down
      auto particle = d_particles.col(i);
      writer.Double(particle.x()); // x
      writer.Double(particle.y()); // y
      writer.Double(particle.z()); // theta
      writer.Double(particle.w()); // weight

      writer.EndArray();
    }
  }
  writer.EndArray();
}
