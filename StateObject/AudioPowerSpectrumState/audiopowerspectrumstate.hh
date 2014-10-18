#pragma once

#include "../stateobject.hh"

#include <vector>

namespace bold
{
  class AudioPowerSpectrumState : public StateObject
  {
  public:
    AudioPowerSpectrumState(double maxFrequency, std::vector<float> dbs);

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer> &writer) const override;

    uint getIndexForFreqHz(double frequencyHz) const;

    uint getMaxIndex() const { return d_dbs.size() - 1; }

    float getDecibelsByIndex(uint index) const { return d_dbs[index]; }

  private:
    double d_maxFrequency;
    std::vector<float> d_dbs;
  };
}