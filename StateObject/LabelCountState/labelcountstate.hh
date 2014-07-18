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

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    std::map<std::shared_ptr<PixelLabel>, uint> d_labelCounts;
  };
}
