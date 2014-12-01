#pragma once

#include "../stateobject.hh"

#include <vector>
#include <cmath>

namespace bold
{
  class AudioPowerSpectrumState : public StateObject
  {
  public:
    AudioPowerSpectrumState(double maxFrequency, std::vector<float> dbs);

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }

    uint getIndexForFreqHz(double frequencyHz) const;

    uint getMaxIndex() const { return d_dbs.size() - 1; }

    float getDecibelsByIndex(uint index) const { return d_dbs[index]; }

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    double d_maxFrequency;
    std::vector<float> d_dbs;
  };

  template<typename TBuffer>
  inline void AudioPowerSpectrumState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
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
            writer.Int(-200);
          else
            writer.Double(db, "%.3f");
        }
      }
      writer.EndArray();
    }
    writer.EndObject();
  }
}