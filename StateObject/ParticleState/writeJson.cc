#include "particlestate.hh"

using namespace bold;
using namespace rapidjson;

void ParticleState::writeJson(Writer<StringBuffer>& writer) const
{
  auto swapNaN = [](double d, double nanVal) -> double { return std::isnan(d) ? nanVal : d; };

  writer.StartObject();
  {
    writer.String("particles");
    writer.StartArray();
    {
      for (int i = 0; i < d_particles.cols(); ++i)
      {
        writer.StartArray();

        auto particle = d_particles.col(i);
        writer.Double(particle.x(), "%.3f"); // x
        writer.Double(particle.y(), "%.3f"); // y
        writer.Double(particle.z(), "%.3f"); // theta
        writer.Double(swapNaN(particle.w(), 0)); // weight

        writer.EndArray();
      }
    }
    writer.EndArray();

    writer.String("pnwsum").Double(d_preNormWeightSum);
  }
  writer.EndObject();
}
