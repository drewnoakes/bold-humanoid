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

    if (I == 0) { r = V; g = K; b = M; }
    if (I == 1) { r = N; g = V; b = M; }
    if (I == 2) { r = M; g = V; b = K; }
    if (I == 3) { r = M; g = N; b = V; }
    if (I == 4) { r = K; g = M; b = V; }
    if (I == 5) { r = V; g = M; b = N; }
  }

  return Colour::bgr(b * 255, g * 255, r * 255);
}