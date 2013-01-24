#include "lutbuilder.ih"

hsv LUTBuilder::bgr2hsv(bgr const& in)
{
  hsv         out;
  int         min, max, delta;

  min = in.r < in.g ? in.r : in.g;
  min = min  < in.b ? min  : in.b;

  max = in.r > in.g ? in.r : in.g;
  max = max  > in.b ? max  : in.b;

  out.v = max;                                // v
  delta = max - min;
  if( max > 0 ) {
    out.s = ((delta << 8) / max);                  // s
  } else {
    // r = g = b = 0                        // s = 0, v is undefined
    out.s = 0;
    out.h = 0;
    out.v = 0;// its now undefined
    return out;
  }
  if (delta ==0)
  {
    out.h = 0;
    return out;
  }

  // 0-63, 64-127, 128- 191
  if( in.r == max )                           // > is bogus, just keeps compilor happy
    out.h = (32 + (in.g - in.b) << 5) / delta;        // between yellow & magenta
  else
    if( in.g == max )
      out.h = 96 + ((in.b - in.r) << 5) / delta;  // between cyan & yellow
    else
      out.h = 160 + ((in.r - in.g) << 5) / delta;  // between magenta & cyan

  return out;
}

