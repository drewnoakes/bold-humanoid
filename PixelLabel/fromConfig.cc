#include "pixellabel.hh"
#include "../Configurable/configurable.hh"

using namespace bold;

PixelLabel PixelLabel::fromConfig(
  std::string objectName,
  int hue,        int hueRange,
  int saturation, int saturationRange,
  int value,      int valueRange
  )
{
  Colour::hsvRange hsvRange;
  hsvRange.h      = Configurable::getParam("vision", objectName + ".hue", hue);
  hsvRange.hRange = Configurable::getParam("vision", objectName + ".hueRange", hueRange);
  hsvRange.s      = Configurable::getParam("vision", objectName + ".saturation", saturation);
  hsvRange.sRange = Configurable::getParam("vision", objectName + ".saturationRange", saturationRange);
  hsvRange.v      = Configurable::getParam("vision", objectName + ".value", value);
  hsvRange.vRange = Configurable::getParam("vision", objectName + ".valueRange", valueRange);

  return PixelLabel(hsvRange, objectName);
}
