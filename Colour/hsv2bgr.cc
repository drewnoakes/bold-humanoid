#include "colour.hh"

using namespace bold;

Colour::bgr Colour::hsv2bgr(hsv const& in)
{
  float b, g, r;

  // All values are in the range [0.0 .. 1.0]
  float S, H, V, F, M, N, K;
  int   I;

  S = in.s / 255.0;
  H = in.h / 255.0;
  V = in.v / 255.0;

  if (S == 0.0)
  {
    // Achromatic case, set level of grey
    r = V;
    g = V;
    b = V;
  }
  else
  {
    // Determine levels of primary colours.
    if (H >= 1.0)
    {
      H = 0.0;
    }
    else
    {
      H = H * 6;
    }
    I = (int) H;   /* should be in the range 0..5 */
    F = H - I;     /* fractional part */

    M = V * (1 - S);
    N = V * (1 - S * F);
    K = V * (1 - S * (1 - F));

    assert (I >= 0 && I <= 5);

    switch (I)
    {
    case 0:
      r = V; g = K; b = M;
      break;
    case 1:
      r = N; g = V; b = M;
      break;
    case 2:
      r = M; g = V; b = K;
      break;
    case 3:
      r = M; g = N; b = V;
      break;
    case 4:
      r = K; g = M; b = V;
      break;
    case 5:
      r = V; g = M; b = N;
      break;
    default:
      // Should never hit this
      assert(false && "IMPOSSIBLE CASE");
      r = g = b = 0;
    }
  }

  return Colour::bgr(b * 255, g * 255, r * 255);
}
