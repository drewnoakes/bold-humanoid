#pragma once

#include "../stateobject.hh"
#include "../../Colour/colour.hh"

namespace bold
{
  class LabelTeacherState : public StateObject
  {
  public:
    LabelTeacherState(Colour::hsvRange selectedRange)
      : d_selectedRange(selectedRange)
    {}

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    Colour::hsvRange d_selectedRange;
  };


  template<typename TBuffer>
  void LabelTeacherState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
  {
    writer.StartObject();
    {
      writer.String("selected-range");
      writer.StartObject();
      {
        writer.String("h");
        writer.StartArray();
        {
          writer.Double(d_selectedRange.hMin);
          writer.Double(d_selectedRange.hMax);
        }
        writer.EndArray();
        writer.String("s");
        writer.StartArray();
        {
          writer.Double(d_selectedRange.sMin);
          writer.Double(d_selectedRange.sMax);
        }
        writer.EndArray();
        writer.String("v");
        writer.StartArray();
        {
          writer.Double(d_selectedRange.sMin);
          writer.Double(d_selectedRange.sMax);
        }
        writer.EndArray();
      }
      writer.EndObject();
    }
    writer.EndObject();
  }
}
