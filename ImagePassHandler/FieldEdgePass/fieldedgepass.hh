#pragma once

#include <memory>
#include <vector>

#include "../imagepasshandler.hh"

namespace bold
{
  class PixelLabel;
  template<typename T> class Setting;

  class FieldEdgePass : public ImagePassHandler<uchar>
  {
  public:
    FieldEdgePass(std::shared_ptr<PixelLabel> fieldLabel, ushort pixelWidth, ushort pixelHeight);

    virtual ~FieldEdgePass() = default;

    virtual ushort getEdgeYValue(ushort x) const = 0;

  protected:
    static void applyConvexHull(std::vector<ushort>& points);

    Setting<bool>* d_useConvexHull;
    std::shared_ptr<PixelLabel> d_fieldLabel;
    ushort d_pixelWidth;
    ushort d_pixelHeight;
    ushort d_minVerticalRunLength;
  };
}
