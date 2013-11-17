#include "pixellabel.hh"
#include "../Configurable/configurable.hh"

using namespace bold;

PixelLabel PixelLabel::fromConfig(
  std::string objectName,
  uchar hMin, uchar hMax,
  uchar sMin, uchar sMax,
  uchar vMin, uchar vMax)
{
  Colour::hsvRange hsvRange;

  hsvRange.hMin = Configurable::getParam("vision", objectName + ".hue.min", hMin);
  hsvRange.hMax = Configurable::getParam("vision", objectName + ".hue.max", hMax);
  hsvRange.sMin = Configurable::getParam("vision", objectName + ".sat.min", sMin);
  hsvRange.sMax = Configurable::getParam("vision", objectName + ".sat.max", sMax);
  hsvRange.vMin = Configurable::getParam("vision", objectName + ".val.min", vMin);
  hsvRange.vMax = Configurable::getParam("vision", objectName + ".val.max", vMax);

  return PixelLabel(hsvRange, objectName);
}
