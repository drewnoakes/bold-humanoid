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
  hsvRange.h      = Configurable::getConfImpl()->getParam(std::string("vision.") + objectName + ".hue", hue);
  hsvRange.hRange = Configurable::getConfImpl()->getParam(std::string("vision.") + objectName + ".hueRange", hueRange);
  hsvRange.s      = Configurable::getConfImpl()->getParam(std::string("vision.") + objectName + ".saturation", saturation);
  hsvRange.sRange = Configurable::getConfImpl()->getParam(std::string("vision.") + objectName + ".saturationRange", saturationRange);
  hsvRange.v      = Configurable::getConfImpl()->getParam(std::string("vision.") + objectName + ".value", value);
  hsvRange.vRange = Configurable::getConfImpl()->getParam(std::string("vision.") + objectName + ".valueRange", valueRange);

  return PixelLabel(hsvRange, objectName);
}
