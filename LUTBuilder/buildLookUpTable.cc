#include "lutbuilder.ih"

uchar* LUTBuilder::buildLookUpTableBGR24(vector<PixelLabel> const& labels)
{
  uchar* LUT = new uchar[1<<24];
  uchar* p = LUT;

  for (int b = 0; b < 256; ++b)
    for (int g = 0; g < 256; ++g)
      for (int r = 0; r < 256; ++r)
          *(p++) = labelPixel(labels, Colour::bgr(b, g, r));

  return LUT;
}

uchar* LUTBuilder::buildLookUpTableBGR18(vector<PixelLabel> const& labels)
{
  uchar* LUT = new uchar[1<<18];
  uchar* p = LUT;

  for (int b = 0; b < 64; ++b)
    for (int g = 0; g < 64; ++g)
      for (int r = 0; r < 64; ++r)
          *(p++) = labelPixel(labels, Colour::bgr(b<<2, g<<2, r<<2));

  return LUT;
}

uchar* LUTBuilder::buildLookUpTableYCbCr18(vector<PixelLabel> const& labels)
{
  uchar* LUT = new uchar[1<<18];
  uchar* p = LUT;

  for (int b = 0; b < 64; ++b)
    for (int g = 0; g < 64; ++g)
      for (int r = 0; r < 64; ++r)
          *(p++) = labelPixel(labels, Colour::YCbCr(b<<2, g<<2, r<<2).toBgrInt());

  return LUT;
}

uchar LUTBuilder::labelPixel(vector<PixelLabel> const& labels, Colour::bgr const& bgr)
{
  for (PixelLabel const& label : labels)
  {
    Colour::hsvRange range = label.hsvRange();
    Colour::hsv hsv = Colour::bgr2hsv(bgr);

    if (range.contains(hsv))
    {
      return label.id();
    }
  }

  return 0;
}