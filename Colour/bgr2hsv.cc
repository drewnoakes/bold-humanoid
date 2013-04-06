#include "colour.ih"

#include <iostream>

Colour::hsv Colour::bgr2hsv(bgr const& in)
{
  Colour::hsv out;
  int         min, max, chroma;

  int const   nh = 255;         // Maximum hue value
  int const   hstep = nh / 3;   // Hue step size between red -> green -> blue

  min = in.r < in.g ? in.r : in.g;
  min = min  < in.b ? min  : in.b;

  max = in.r > in.g ? in.r : in.g;
  max = max  > in.b ? max  : in.b;

  out.v = max;                          // v
  chroma = max - min;
  if (max > 0)
  {
    out.s = 255 * chroma / max;       // s
  }
  else
  {
    // r = g = b = 0                    // s = 0, v is undefined
    out.s = 0;
    out.h = 0;
    out.v = 0; // it's now undefined
    return out;
  }

  if (chroma == 0)
  {
    out.h = 0;
    return out;
  }

  const int chroma2 = chroma * 2;
  int offset;
  int diff;

  if (in.r == max)
  {
    offset = 3 * hstep;
    diff = in.g - in.b;
  }
  else if (in.g == max)
  {
    offset = hstep;
    diff = in.b - in.r;
  }
  else
  {
    offset = 2 * hstep;
    diff = in.r - in.g;
  }

  int h = offset + (diff * (hstep + 1)) / chroma2;

  // Rotate such that red has hue 0
  if (h >= nh)
    h -= nh;

  assert(h >= 0 && h < 256);

  out.h = h;

  return out;
}
