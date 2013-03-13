#include "colour.hh"

using namespace bold;
using namespace std;
using namespace cv;


Colour::YCbCr::YCbCr()
{}

Colour::YCbCr::YCbCr(uchar y, uchar cb, uchar cr)
: y(y), cb(cb), cr(cr)
{
//assert(y >= 16);
//assert(y <= 235);
//assert(cb >= 16);
//assert(cb <= 240);
//assert(cr >= 16);
//assert(cr <= 240);
}

bool Colour::YCbCr::isValid() const
{
  return y >= 16 && y <= 235
      && cb >= 16 && cb <= 240
      && cr >= 16 && cr <= 240;
}

Colour::bgr Colour::YCbCr::toBgrInt() const
{
//assert(isValid());

  int y = this->y - 16;
  int cb = this->cb - 128;
  int cr = this->cr - 128;

  int r = (298 * y + 409 * cr + 128) >> 8;
  int g = (298 * y - 100 * cb - 208 * cr) >> 8;
  int b = (298 * y + 516 * cb + 128) >> 8;

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
//assert(isValid());

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
