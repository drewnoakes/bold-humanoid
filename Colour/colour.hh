#pragma once

#include <vector>
#include <string>
#include <Eigen/Core>
#include <opencv2/core/core.hpp>

namespace bold
{
  namespace Colour
  {
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

      Eigen::Vector3d toRgbUnitVector() const
      {
        return Eigen::Vector3d(r/255.0, g/255.0, b/255.0);
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

    class hsv;
    class bgr;

    hsv bgr2hsv(bgr const& in);
    bgr hsv2bgr(hsv const& in);

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

      bgr toBgr() const
      {
        return hsv2bgr(*this);
      }

      uchar h;
      uchar s;
      uchar v;
    };

    struct hsvRange
    {
      uchar hMin;
      uchar hMax;
      uchar sMin;
      uchar sMax;
      uchar vMin;
      uchar vMax;

      hsvRange();

      hsvRange(
        uchar hMin, uchar hMax,
        uchar sMin, uchar sMax,
        uchar vMin, uchar vMax);

      uchar getHMid() const;
      uchar getSMid() const;
      uchar getVMid() const;

      /** Obtain the center colour for the range, in BGR format. */
      Colour::bgr toBgr() const;

      bool contains(hsv const& hsv) const;

      bool isValid() const;

      static hsvRange fromBytes(uchar hMin, uchar hMax, uchar sMin, uchar sMax, uchar vMin, uchar vMax);
      static hsvRange fromDoubles(double hMin, double hMax, double sMin, double sMax, double vMin, double vMax);

      hsvRange withHMin(uchar value) const { return hsvRange(value, hMax, sMin, sMax, vMin, vMax); }
      hsvRange withHMax(uchar value) const { return hsvRange(hMin, value, sMin, sMax, vMin, vMax); }
      hsvRange withSMin(uchar value) const { return hsvRange(hMin, hMax, value, sMax, vMin, vMax); }
      hsvRange withSMax(uchar value) const { return hsvRange(hMin, hMax, sMin, value, vMin, vMax); }
      hsvRange withVMin(uchar value) const { return hsvRange(hMin, hMax, sMin, sMax, value, vMax); }
      hsvRange withVMax(uchar value) const { return hsvRange(hMin, hMax, sMin, sMax, vMin, value); }
    };

    inline void yCbCrToBgrInPlace(uchar* pxl)
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
