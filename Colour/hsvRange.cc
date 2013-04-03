#include "colour.hh"

using namespace bold;
using namespace std;
using namespace cv;

Colour::hsvRange::hsvRange()
{}

Colour::hsvRange::hsvRange(
  int hue,        int hueRange,
  int saturation, int saturationRange,
  int value,      int valueRange)
: h(hue),         hRange(hueRange),
  s(saturation),  sRange(saturationRange),
  v(value),       vRange(valueRange)
{}

/** Obtain the center colour for the range, in BGR format. */
Colour::bgr Colour::hsvRange::toBgr() const
{
  return Colour::hsv2bgr(Colour::hsv(h, s, v));
}

bool Colour::hsvRange::contains(Colour::hsv const& hsv) const
{
  int hDiff = abs(hsv.h - h);
  hDiff = std::min(hDiff, 192 - hDiff);

  return (hDiff <= hRange &&
          hsv.s >= s - sRange && hsv.s <= s + sRange &&
          hsv.v >= v - vRange && hsv.v <= v + vRange);
}

Colour::hsvRange Colour::hsvRange::fromBytes(uchar h, uchar hRange, uchar s, uchar sRange, uchar v, uchar vRange)
{
  return hsvRange(h, hRange, s, sRange, v, vRange);
}

Colour::hsvRange Colour::hsvRange::fromDoubles(double h, double hRange, double s, double sRange, double v, double vRange)
{
  assert(h >= 0.0 && h <= 360.0);
  assert(hRange >= 0.0 && hRange <= 360.0);
  assert(s >= 0.0 && sRange <= 1.0);
  assert(sRange >= 0.0 && sRange <= 1.0);
  assert(v >= 0.0 && vRange <= 1.0);
  assert(vRange >= 0.0 && vRange <= 1.0);

  return hsvRange((h/360.0)*255, (hRange/360)*255, s*255, sRange*255, v*255, vRange*255);
}
