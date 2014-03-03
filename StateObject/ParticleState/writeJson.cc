#include "particlestate.hh"

using namespace bold;
using namespace rapidjson;

void ParticleState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  writer.String("particles").StartArray();
  {
    for (unsigned i = 0; i < d_particles.cols(); ++i)
    {
      writer.StartArray();

      auto particle = d_particles.col(i);
      writer.Double(particle.x(), "%.3f"); // x
      writer.Double(particle.y(), "%.3f"); // y
      writer.Double(particle.z(), "%.3f"); // theta
      writer.Double(isnan(particle.w()) ? 0 : particle.w()); // weight

      writer.EndArray();
    }
  }
  writer.EndArray();
  writer.String("pnwsum").Double(d_preNormWeightSum);
  writer.EndObject();
}
