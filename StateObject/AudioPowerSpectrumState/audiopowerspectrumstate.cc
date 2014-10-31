#include "audiopowerspectrumstate.hh"

#include <cmath>

using namespace bold;
using namespace rapidjson;
using namespace std;

AudioPowerSpectrumState::AudioPowerSpectrumState(double maxFrequency, std::vector<float> dbs)
: d_maxFrequency(maxFrequency),
  d_dbs(dbs)
{}

uint AudioPowerSpectrumState::getIndexForFreqHz(double frequencyHz) const
{
  return (uint) (frequencyHz * d_dbs.size() / d_maxFrequency);
}
