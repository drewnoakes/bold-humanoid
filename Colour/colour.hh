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

      hsvRange()
      {};

      hsvRange(
        int hue,        int hueRange,
        int saturation, int saturationRange,
        int value,      int valueRange)
      : h(hue),         hRange(hueRange),
        s(saturation),  sRange(saturationRange),
        v(value),       vRange(valueRange)
      {}

      /** Obtain the center colour for the range, in BGR format. */
      Colour::bgr toBgr() const
      {
        return Colour::hsv2bgr(Colour::hsv(h, s, v));
      }

      inline bool contains(hsv hsv) const
      {
        int hDiff = abs(hsv.h - h);
        hDiff = std::min(hDiff, 192 - hDiff);

        return (hDiff <= hRange &&
                hsv.s >= s - sRange && hsv.s <= s + sRange &&
                hsv.v >= v - vRange && hsv.v <= v + vRange);
      }

      static hsvRange fromBytes(uchar h, uchar hRange, uchar s, uchar sRange, uchar v, uchar vRange)
      {
        return hsvRange(h, hRange, s, sRange, v, vRange);
      }

      static hsvRange fromDoubles(double h, double hRange, double s, double sRange, double v, double vRange)
      {
        assert(h >= 0.0 && h <= 360.0);
        assert(hRange >= 0.0 && hRange <= 360.0);
        assert(s >= 0.0 && sRange <= 1.0);
        assert(sRange >= 0.0 && sRange <= 1.0);
        assert(v >= 0.0 && vRange <= 1.0);
        assert(vRange >= 0.0 && vRange <= 1.0);

        return hsvRange((h/360.0)*255, (hRange/360)*255, s*255, sRange*255, v*255, vRange*255);
      }
    };

    static hsv bgr2hsv(bgr const& in);
    static bgr hsv2bgr(hsv const& in);
  };

  std::ostream& operator<<(std::ostream &stream, Colour::hsv const& hsv);
  std::ostream& operator<<(std::ostream &stream, Colour::hsvRange const& hsv);
}

#endif
