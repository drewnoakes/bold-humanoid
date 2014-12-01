#pragma once

#include <vector>
#include <map>
#include <memory>

#include "../stateobject.hh"
#include "../../PixelLabel/pixellabel.hh"

namespace bold
{
  enum class LabelClass : uint8_t;

  class LabelCountState : public StateObject
  {
  public:
    LabelCountState(std::map<std::shared_ptr<PixelLabel>, uint> labelCounts)
    : d_labelCounts(labelCounts)
    {}

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    std::map<std::shared_ptr<PixelLabel>, uint> d_labelCounts;
  };

  template<typename TBuffer>
  inline void LabelCountState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
  {
    writer.StartObject();
    {
      writer.String("labels");
      writer.StartArray();
      {
        for (auto const& pair : d_labelCounts)
        {
          auto label = pair.first;
          uint const& count = pair.second;
          writer.StartObject();
          {
            writer.String("name");
            writer.String(label->getName().c_str());
            writer.String("id");
            writer.Uint((uint8_t)label->getID());
            writer.String("count");
            writer.Uint(count);
          }
          writer.EndObject();
        }
      }
      writer.EndArray();
    }
    writer.EndObject();
  }
}
