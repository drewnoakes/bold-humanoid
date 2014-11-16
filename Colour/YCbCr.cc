#include "colour.hh"

using namespace bold;
using namespace std;

Colour::YCbCr::YCbCr(uchar y, uchar cb, uchar cr)
: y(y), cb(cb), cr(cr)
{
//ASSERT(y >= 16);
//ASSERT(y <= 235);
//ASSERT(cb >= 16);
//ASSERT(cb <= 240);
//ASSERT(cr >= 16);
//ASSERT(cr <= 240);
}

bool Colour::YCbCr::isValid() const
{
  return y >= 16 && y <= 235
      && cb >= 16 && cb <= 240
      && cr >= 16 && cr <= 240;
}

Colour::bgr Colour::YCbCr::toBgrInt() const
{
//ASSERT(isValid());
//{ 33292, -6472, -9519, 18678 }
  // YUV coefficients
  /*
  int c0 = 33292;
  int c1 = -6472;
  int c2 = -9519;
  int c3 = 18678;
  */

  // YCrCb coefficients
  int c0 = 22987;
  int c1 = -11698;
  int c2 = -5636;
  int c3 = 29049;
  
  int y = this->y;
  int cb = this->cb - 128;
  int cr = this->cr - 128;

  int b = y + (((c3 * cb) + (1 << 13)) >> 14);
  int g = y + (((c2 * cb + c1 * cr) + (1 << 13)) >> 14);
  int r = y + (((c0 * cr) + (1 << 13)) >> 14);

  if (r < 0)
    r = 0;
  else if (r > 255)
    r = 255;

  if (g < 0)
    g = 0;
  else if (g > 255)
    g = 255;

  if (b < 0)
    b = 0;
  else if (b > 255)
    b = 255;

  return Colour::bgr(b, g, r);
}

Colour::bgr Colour::YCbCr::toBgrFloat() const
{
//ASSERT(isValid());

  float y = this->y;
  float cb = this->cb;
  float cr = this->cr;

  int r = y + 1.40200 * (cr - 0x80);
  int g = y - 0.34414 * (cb - 0x80) - 0.71414 * (cr - 0x80);
  int b = y + 1.77200 * (cb - 0x80);

  if (r < 0)
    r = 0;
  else if (r > 255)
    r = 255;

  if (g < 0)
    g = 0;
  else if (g > 255)
    g = 255;

  if (b < 0)
    b = 0;
  else if (b > 255)
    b = 255;

  return Colour::bgr(b, g, r);
}
