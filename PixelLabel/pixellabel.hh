#pragma once

#include <opencv2/core/core.hpp>
#include <string>
#include <cassert>

#include "../Colour/colour.hh"

namespace bold
{
  class PixelLabel
  {
  private:
    static uchar nextId;

    uchar d_id;
    Colour::hsvRange d_hsvRange;
    std::string d_name;

  public:
    static PixelLabel fromConfig(
      std::string objectName,
      int hue,        int hueRange,
      int saturation, int saturationRange,
      int value,      int valueRange
      );

    PixelLabel() {}

    PixelLabel(Colour::hsvRange hsvRange, std::string name)
    : d_id(nextId++),
      d_hsvRange(hsvRange),
      d_name(name)
    {
      assert(d_id != 0);
    }

    uchar id() const { return d_id; }
    Colour::hsvRange hsvRange() const { return d_hsvRange; }
    std::string name() const { return d_name; }

    void setHsvRange(Colour::hsvRange const& hsvRange) { d_hsvRange = hsvRange; }

    bool operator<(const PixelLabel& other) const
    {
      return d_id < other.d_id;
    }
  };

  std::ostream& operator<<(std::ostream &stream, bold::PixelLabel const& pixelLabel);
}
