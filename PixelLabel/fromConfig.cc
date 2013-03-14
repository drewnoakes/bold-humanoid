#include "pixellabel.hh"

using namespace bold;

PixelLabel PixelLabel::fromConfig(
  minIni const& ini,
  std::string objectName,
  int hue,        int hueRange,
  int saturation, int saturationRange,
  int value,      int valueRange
  )
{
  Colour::hsvRange hsvRange;
  hsvRange.h      = ini.geti("Vision", objectName + "Hue",             hue);
  hsvRange.hRange = ini.geti("Vision", objectName + "HueRange",        hueRange);
  hsvRange.s      = ini.geti("Vision", objectName + "Saturation",      saturation);
  hsvRange.sRange = ini.geti("Vision", objectName + "SaturationRange", saturationRange);
  hsvRange.v      = ini.geti("Vision", objectName + "Value",           value);
  hsvRange.vRange = ini.geti("Vision", objectName + "ValueRange",      valueRange);

  return PixelLabel(hsvRange, objectName);
}
