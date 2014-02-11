#include "colour.ih"

using namespace std;

Colour::bgr Colour::hsv2bgr(hsv const& in)
{
  Colour::bgr out;

  int const   nh = 256;         // Maximum hue value
  int const   hstep = nh / 3;   // Hue step size between red -> green -> blue

  int r, g, b;

  if (in.s == 0)
  {
    // Achromatic case, set level of grey
    r = g = b = in.v;
  }
  else
  {
    int chroma = in.s * in.v / 255;

    int I = 2 * in.h / hstep; // [0-5]
    int F = in.h % hstep; // [0-hstep)

    assert (I >= 0 && I <= 5);

    int X = chroma * (255 - abs(2 * 255 * F / hstep - 255)) / 255;

    switch (I)
    {
    case 0:
      r = chroma;
      g = X;
      b = 0;
      break;
    case 1:
      r = X;
      g = chroma;
      b = 0;
      break;
    case 2:
      r = 0;
      g = chroma;
      b = X;
      break;
    case 3:
      r = 0;
      g = X;
      b = chroma;
      break;
    case 4:
      r = X;
      g = 0;
      b = chroma;
      break;
    case 5:
      r = chroma;
      g = 0;
      b = X;
      break;
    default:
      // Should never hit this
      assert(false && "IMPOSSIBLE CASE");
      r = g = b = 0;
    }

    int M = in.v - chroma;
    r += M;
    g += M;
    b += M;
  }

  return Colour::bgr(b, g, r);
}
