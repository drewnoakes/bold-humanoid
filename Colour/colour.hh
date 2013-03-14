#ifndef BOLD_COLOUR_HH
#define BOLD_COLOUR_HH

#include <vector>
#include <string>
#include <opencv2/core/core.hpp>

namespace bold
{
  class Colour
  {
  public:
    struct bgr
    {
      bgr()
      {}

      bgr(uchar b, uchar g, uchar r)
        : b(b), g(g), r(r)
      {}

      bgr invert() const
      {
        return bgr(255-b, 255-g, 255-r);
      }

      cv::Scalar toScalar() const
      {
        return cv::Scalar(b, g, r);
      }

      uchar b;
      uchar g;
      uchar r;
    };

    struct YCbCr
    {
      // http://www.equasys.de/colorconversion.html

      YCbCr();
      YCbCr(uchar y, uchar cb, uchar cr);

      bool isValid() const;

      Colour::bgr toBgrInt() const;
      Colour::bgr toBgrFloat() const;

      uchar y;
      uchar cb;
      uchar cr;
    };

    struct hsv
    {
      hsv()
      {}

      hsv(int h, int s, int v)
        : h(h), s(s), v(v)
      {}

      // TODO why are these int?
      int h;
      int s;
      int v;
    };

    struct hsvRange
    {
      int h;
      int hRange;
      int s;
      int sRange;
      int v;
      int vRange;

      hsvRange();

      hsvRange(
        int hue,        int hueRange,
        int saturation, int saturationRange,
        int value,      int valueRange);

      /** Obtain the center colour for the range, in BGR format. */
      Colour::bgr toBgr() const;

      bool contains(hsv const& hsv) const;

      static hsvRange fromBytes(uchar h, uchar hRange, uchar s, uchar sRange, uchar v, uchar vRange);
      static hsvRange fromDoubles(double h, double hRange, double s, double sRange, double v, double vRange);
    };

    static hsv bgr2hsv(bgr const& in);
    static bgr hsv2bgr(hsv const& in);

    static void yCbCrToBgbInPlace(unsigned char* pxl)
    {
      Colour::YCbCr* ycbcr = reinterpret_cast<Colour::YCbCr*>(pxl);
      Colour::bgr* bgr = reinterpret_cast<Colour::bgr*>(pxl);
      *bgr = (*ycbcr).toBgrInt();
    }
  };

  std::ostream& operator<<(std::ostream &stream, Colour::hsv const& hsv);
  std::ostream& operator<<(std::ostream &stream, Colour::hsvRange const& hsv);
}

#endif
