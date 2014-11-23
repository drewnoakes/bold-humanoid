#include "colour.hh"

#include "../util/assert.hh"

using namespace bold;
using namespace std;
using namespace cv;

Colour::hsvRange::hsvRange()
: hMin(0), hMax(0),
  sMin(0), sMax(0),
  vMin(0), vMax(0)
{}

Colour::hsvRange::hsvRange(
  uchar hMin, uchar hMax,
  uchar sMin, uchar sMax,
  uchar vMin, uchar vMax)
: hMin(hMin), hMax(hMax),
  sMin(sMin), sMax(sMax),
  vMin(vMin), vMax(vMax)
{
  // NOTE if hMin > hMax, then the hue value wraps around 360 degrees (255 in 8-bit form)
  ASSERT(sMin <= sMax);
  ASSERT(vMin <= vMax);
}

Colour::hsv Colour::hsvRange::toHsv() const
{
  return Colour::hsv(getHMid(), getSMid(), getVMid());
}

/** Obtain the center colour for the range, in BGR format. */
Colour::bgr Colour::hsvRange::toBgr() const
{
  return toHsv().toBgr();
}

uchar Colour::hsvRange::getHMid() const
{
  // The hue midpoint is a little trickier than the S or V midpoints, as
  // hue may wrap around
  if (hMin <= hMax)
  {
    return hMin + ((hMax-hMin)/2);
  }

  int hMinInt = hMin;
  int hMaxInt = hMax;

  int mid = hMaxInt + ((hMinInt+255-hMaxInt)/2);
  while (mid > 255)
    mid -= 255;
  ASSERT(mid >= 0);
  return mid;
}

uchar Colour::hsvRange::getSMid() const
{
  return sMin + ((sMax-sMin)/2);
}

uchar Colour::hsvRange::getVMid() const
{
  return vMin + ((vMax-vMin)/2);
}

bool Colour::hsvRange::contains(Colour::hsv const& hsv) const
{
  if (hMin <= hMax)
  {
    // Hue is not wrapped
    if (hsv.h < hMin || hsv.h > hMax)
      return false;
  }
  else
  {
    // Hue is wrapped
    if (hsv.h < hMin && hsv.h > hMax)
      return false;
  }

  return hsv.s >= sMin && hsv.s <= sMax &&
         hsv.v >= vMin && hsv.v <= vMax;
}

bool Colour::hsvRange::isValid() const
{
  return sMin <= sMax && vMin <= vMax;
}

Colour::hsvRange Colour::hsvRange::fromBytes(uchar hMin, uchar hMax, uchar sMin, uchar sMax, uchar vMin, uchar vMax)
{
  return hsvRange(hMin, hMax, sMin, sMax, vMin, vMax);
}

Colour::hsvRange Colour::hsvRange::fromDoubles(double hMin, double hMax, double sMin, double sMax, double vMin, double vMax)
{
  ASSERT(hMin >= 0.0 && hMin <= 360.0);
  ASSERT(hMax >= 0.0 && hMax <= 360.0);
  ASSERT(sMin >= 0.0 && sMin <= 1.0);
  ASSERT(sMax >= 0.0 && sMax <= 1.0);
  ASSERT(vMin >= 0.0 && vMin <= 1.0);
  ASSERT(vMax >= 0.0 && vMax <= 1.0);

  return hsvRange((hMin/360.0)*255, (hMax/360)*255, sMin*255, sMax*255, vMin*255, vMax*255);
}

Colour::hsvRange Colour::hsvRange::containing(hsv const& hsv)
{
  hsvRange res = *this;
  if ((hMin <= hMax && (hsv.h < hMin || hsv.h > hMax)) ||
      (hMin > hMax && (hsv.h < hMin && hsv.h > hMax)))
  {
    auto d1 = Math::angleDiffRads(hsv.h * M_PI / 128.0, hMax * M_PI / 128.0);
    auto d2 = Math::angleDiffRads(hMin * M_PI / 128.0, hsv.h * M_PI / 128.0);
    if (d1 < d2)
      res.hMin = hsv.h;
    else
      res.hMax = hsv.h;
  }

  res.sMin = std::min(sMin, hsv.s);
  res.sMax = std::max(sMax, hsv.s);
  res.vMin = std::min(vMin, hsv.v);
  res.vMax = std::max(vMax, hsv.v);
  return res;
}
