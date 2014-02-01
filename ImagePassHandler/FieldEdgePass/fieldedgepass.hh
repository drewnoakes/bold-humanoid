#pragma once

#include <memory>
#include <vector>

#include "../imagepasshandler.hh"
#include "../../Config/config.hh"

namespace bold
{
  class PixelLabel;

  class FieldEdgePass : public ImagePassHandler<uchar>
  {
  public:
    FieldEdgePass(std::shared_ptr<PixelLabel> fieldLabel, ushort pixelWidth, ushort pixelHeight)
    : d_fieldLabel(fieldLabel),
      d_pixelWidth(pixelWidth),
      d_pixelHeight(pixelHeight)
    {
      Config::getSetting<int>("vision.field-edge-pass.min-vertical-run-length")->track([this](int value) { d_minVerticalRunLength = value; });
    }

    virtual ~FieldEdgePass() = default;

    virtual ushort getEdgeYValue(ushort x) const = 0;

  protected:
    std::shared_ptr<PixelLabel> d_fieldLabel;
    ushort d_pixelWidth;
    ushort d_pixelHeight;
    ushort d_minVerticalRunLength;
  };
}
