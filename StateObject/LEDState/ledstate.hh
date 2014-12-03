#pragma once

#include "../stateobject.hh"
#include "../../Colour/colour.hh"

namespace bold
{
  class LEDState : public StateObject
  {
  public:
    LEDState(Colour::bgr eyeColour, Colour::bgr foreheadColour, bool redLed, bool greenLed, bool blueLed)
    : d_eyeColour(eyeColour),
      d_foreheadColour(foreheadColour),
      d_redLed(redLed),
      d_greenLed(greenLed),
      d_blueLed(blueLed)
    {}

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    Colour::bgr d_eyeColour;
    Colour::bgr d_foreheadColour;
    bool d_redLed;
    bool d_greenLed;
    bool d_blueLed;
  };

  template<typename TBuffer>
  inline void LEDState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
  {
    writer.StartObject();
    {
      writer.String("eyeColour");
      writer.StartArray();
      {
        writer.Int(d_eyeColour.r);
        writer.Int(d_eyeColour.g);
        writer.Int(d_eyeColour.b);
      }
      writer.EndArray();
      writer.String("foreheadColour");
      writer.StartArray();
      {
        writer.Int(d_foreheadColour.r);
        writer.Int(d_foreheadColour.g);
        writer.Int(d_foreheadColour.b);
      }
      writer.EndArray();
      writer.String("led");
      writer.StartArray();
      {
        writer.Bool(d_redLed);
        writer.Bool(d_greenLed);
        writer.Bool(d_blueLed);
      }
      writer.EndArray();
    }
    writer.EndObject();
  }
}
