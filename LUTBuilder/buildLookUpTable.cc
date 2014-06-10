#include "lutbuilder.ih"

shared_ptr<uchar const> LUTBuilder::buildLookUpTableBGR24(vector<shared_ptr<PixelLabel>> const& labels)
{
  uchar* lut = new uchar[1<<24];
  uchar* p = lut;

  for (int b = 0; b < 256; ++b)
    for (int g = 0; g < 256; ++g)
      for (int r = 0; r < 256; ++r)
          *(p++) = labelPixel(labels, Colour::bgr(b, g, r));

  return shared_ptr<uchar const>(lut, [](uchar const* p) { delete[] p; });
}

shared_ptr<uchar const> LUTBuilder::buildLookUpTableBGR18(vector<shared_ptr<PixelLabel>> const& labels)
{
  uchar* lut = new uchar[1<<18];
  uchar* p = lut;

  for (int b = 0; b < 64; ++b)
    for (int g = 0; g < 64; ++g)
      for (int r = 0; r < 64; ++r)
          *(p++) = labelPixel(labels, Colour::bgr(b<<2, g<<2, r<<2));

  return shared_ptr<uchar const>(lut, [](uchar const* p) { delete[] p; });
}

shared_ptr<uchar const> LUTBuilder::buildLookUpTableYCbCr18(vector<shared_ptr<PixelLabel>> const& labels)
{
  uchar* lut = new uchar[1<<18];
  uchar* p = lut;

  // TODO should we use the floating point conversion from YCbCr to BGR here for accuracy?

  for (int y = 0; y < 64; ++y)
    for (int cb = 0; cb < 64; ++cb)
      for (int cr = 0; cr < 64; ++cr)
          *(p++) = labelPixel(labels, Colour::YCbCr(y<<2, cb<<2, cr<<2).toBgrInt());

  return shared_ptr<uchar const>(lut, [](uchar const* p) { delete[] p; });
}

uchar LUTBuilder::labelPixel(vector<shared_ptr<PixelLabel>> const& labels, Colour::bgr const& bgr)
{
  auto const& hsv = Colour::bgr2hsv(bgr);

  // Find first that matches
  for (shared_ptr<PixelLabel> label : labels)
  {
    if (label->hsvRange().contains(hsv))
    {
      return label->id();
    }
  }

  return 0;
}
