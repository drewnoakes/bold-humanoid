#pragma once

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

      bool operator==(bgr const& other) const
      {
        return
          b == other.b &&
          g == other.g &&
          r == other.r;
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

      bool operator==(YCbCr const& other) const
      {
        return
          y  == other.y  &&
          cb == other.cb &&
          cr == other.cr;
      }

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

      bool operator==(hsv const& other) const
      {
        return
          h == other.h &&
          s == other.s &&
          v == other.v;
      }

      uchar h;
      uchar s;
      uchar v;

      static const int hueRange = (255 / 3) * 3; // 255/3 == 85, 85*3 == 255
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

      hsvRange withH(int value)      const { return hsvRange(value, hRange, s, sRange, v, vRange); }
      hsvRange withHRange(int value) const { return hsvRange(h, value, s, sRange, v, vRange); }
      hsvRange withS(int value)      const { return hsvRange(h, hRange, value, sRange, v, vRange); }
      hsvRange withSRange(int value) const { return hsvRange(h, hRange, s, value, v, vRange); }
      hsvRange withV(int value)      const { return hsvRange(h, hRange, s, sRange, value, vRange); }
      hsvRange withVRange(int value) const { return hsvRange(h, hRange, s, sRange, v, value); }
    };

    static hsv bgr2hsv(bgr const& in);
    static bgr hsv2bgr(hsv const& in);

    static void yCbCrToBgrInPlace(uchar* pxl)
    {
      Colour::YCbCr* ycbcr = reinterpret_cast<Colour::YCbCr*>(pxl);
      Colour::bgr* bgr = reinterpret_cast<Colour::bgr*>(pxl);
      *bgr = (*ycbcr).toBgrInt();
    }
  };

  std::ostream& operator<<(std::ostream &stream, Colour::bgr const& bgr);
  std::ostream& operator<<(std::ostream &stream, Colour::hsv const& hsv);
  std::ostream& operator<<(std::ostream &stream, Colour::hsvRange const& hsv);
}
