#include "particlestate.hh"

#include "../../JsonWriter/jsonwriter.hh"

using namespace bold;
using namespace rapidjson;

void ParticleState::writeJson(Writer<StringBuffer>& writer) const
{
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
        writer.Double(atan2(particle(3), particle(2)), "%.3f"); // theta
        JsonWriter::swapNaN(writer, particle(4)); // weight

        writer.EndArray();
      }
    }
    writer.EndArray();

    writer.String("pnwsum");
    writer.Double(d_preNormWeightSum);
    writer.String("pnwsumsmooth");
    writer.Double(d_smoothedPreNormWeightSum);
    writer.String("uncertainty");
    writer.Double(d_uncertainty);
  }
  writer.EndObject();
}
