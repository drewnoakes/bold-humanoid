#pragma once

#include "../stateobject.hh"
#include "../../Drawing/drawing.hh"

#include <memory>
#include <vector>

namespace bold
{
  class DrawingState : public StateObject
  {
  public:
    DrawingState(std::unique_ptr<std::vector<std::unique_ptr<DrawingItem const>>> items)
    : d_drawingItems(std::move(items))
    {}

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    std::unique_ptr<std::vector<std::unique_ptr<DrawingItem const>>> d_drawingItems;
  };
}
