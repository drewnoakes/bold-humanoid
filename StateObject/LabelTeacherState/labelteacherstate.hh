#pragma once

#include "../stateobject.hh"
#include "../../Colour/colour.hh"

namespace bold
{
  class LabelTeacherState : public StateObject
  {
  public:
    LabelTeacherState(Colour::hsvRange selectedRange, std::pair<Colour::hsv, Colour::hsv> selectedDistribution)
      : d_selectedRange(selectedRange),
        d_selectedDistribution(selectedDistribution)
    {}

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    Colour::hsvRange d_selectedRange;
    std::pair<Colour::hsv, Colour::hsv> d_selectedDistribution;
  };


  template<typename TBuffer>
  void LabelTeacherState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
  {
    writer.StartObject();
    {
      writer.String("selectedRange");
      writer.StartObject();
      {
        writer.String("hue");
        writer.StartArray();
        {
          writer.Uint(d_selectedRange.hMin);
          writer.Uint(d_selectedRange.hMax);
        }
        writer.EndArray();
        writer.String("sat");
        writer.StartArray();
        {
          writer.Uint(d_selectedRange.sMin);
          writer.Uint(d_selectedRange.sMax);
        }
        writer.EndArray();
        writer.String("val");
        writer.StartArray();
        {
          writer.Uint(d_selectedRange.vMin);
          writer.Uint(d_selectedRange.vMax);
        }
        writer.EndArray();
      }
      writer.EndObject();

      writer.String("selectedDist");
      writer.StartObject();
      {
        writer.String("hue");
        writer.StartArray();
        {
          writer.Uint(d_selectedDistribution.first.h);
          writer.Uint(d_selectedDistribution.second.h);
        }
        writer.EndArray();
        writer.String("sat");
        writer.StartArray();
        {
          writer.Uint(d_selectedDistribution.first.s);
          writer.Uint(d_selectedDistribution.second.s);
        }
        writer.EndArray();
        writer.String("val");
        writer.StartArray();
        {
          writer.Uint(d_selectedDistribution.first.v);
          writer.Uint(d_selectedDistribution.second.v);
        }
        writer.EndArray();
      }
      writer.EndObject();
    }
    writer.EndObject();
  }
}
