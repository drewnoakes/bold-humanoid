#include "colour.ih"

Colour::hsv Colour::bgr2hsv(bgr const& in)
{
  Colour::hsv out;
  int         min, max, chroma;

  min = in.r < in.g ? in.r : in.g;
  min = min  < in.b ? min  : in.b;

  max = in.r > in.g ? in.r : in.g;
  max = max  > in.b ? max  : in.b;

  out.v = max;                          // v
  chroma = max - min;
  if (max > 0)
  {
    out.s = (((chroma << 8) - chroma) / max);       // s
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

  if (in.r == max)                               // > is bogus, just keeps compiler happy
    out.h = ((in.g - in.b) * 42) / chroma;   // between yellow & magenta
  else if (in.g == max)
    out.h = 85 + ((in.b - in.r) * 42) / chroma;   // between cyan & yellow
  else
    out.h = 170 + ((in.r - in.g) * 42) / chroma;  // between magenta & cyan
  if (out.h < 0)
    out.h += 254;

  return out;
}
