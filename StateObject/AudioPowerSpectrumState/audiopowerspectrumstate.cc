#include "audiopowerspectrumstate.hh"

#include <cmath>

using namespace bold;
using namespace rapidjson;
using namespace std;

AudioPowerSpectrumState::AudioPowerSpectrumState(double maxFrequency, std::vector<float> dbs)
: d_maxFrequency(maxFrequency),
  d_dbs(dbs)
{}

void AudioPowerSpectrumState::writeJson(Writer<StringBuffer> &writer) const
{
  writer.StartObject();
  {
    writer.String("maxHertz");
    writer.Double(d_maxFrequency);
    writer.String("dbLevels");
    writer.StartArray();
    {
      for (float const& db : d_dbs)
      {
        if (std::isinf(db))
          writer.Double(-200);
        else
          writer.Double(db);
      }
    }
    writer.EndArray();
  }
  writer.EndObject();
}

uint AudioPowerSpectrumState::getIndexForFreqHz(double frequencyHz) const
{
  return (uint) (frequencyHz * d_dbs.size() / d_maxFrequency);
}
