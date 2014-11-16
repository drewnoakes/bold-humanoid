#include "colour.hh"

#include "../util/assert.hh"

using namespace bold;

Colour::hsv Colour::bgr2hsv(bgr const& in)
{
  int const hstep = 255 / 3;   // Hue step size between red -> green -> blue

  int min = in.r < in.g ? in.r : in.g;
  min = min  < in.b ? min  : in.b;

  int max = in.r > in.g ? in.r : in.g;
  max = max  > in.b ? max  : in.b;

  int chroma = max - min;

  Colour::hsv out;

  if (max > 0)
  {
    out.s = 255 * chroma / max;         // s
  }
  else
  {
    // r = g = b = 0                    // s = 0, v is undefined
    out.s = 0;
    out.h = 0;
    out.v = 0; // it's now undefined
    return out;
  }

  out.v = max;                          // v

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
  if (h >= 255) // TODO would this be faster using mod?
    h -= 255;

  ASSERT(h >= 0 && h < 256);

  out.h = h;

  return out;
}
