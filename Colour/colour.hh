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
      bgr() = default;

      bgr(uint8_t b, uint8_t g, uint8_t r)
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

      uint8_t b;
      uint8_t g;
      uint8_t r;

      static const bgr black;
      static const bgr grey;
      static const bgr white;
      static const bgr red;
      static const bgr lightRed;
      static const bgr darkRed;
      static const bgr green;
      static const bgr lightGreen;
      static const bgr darkGreen;
      static const bgr blue;
      static const bgr lightBlue;
      static const bgr darkBlue;
      static const bgr orange;
      static const bgr purple;
      static const bgr yellow;
      static const bgr cyan;
      static const bgr magenta;
    };

    struct YCbCr
    {
      // http://www.equasys.de/colorconversion.html

      YCbCr();
      YCbCr(uint8_t y, uint8_t cb, uint8_t cr);

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

      uint8_t y;
      uint8_t cb;
      uint8_t cr;
    };

    struct hsv;

    hsv bgr2hsv(bgr const& in);
    bgr hsv2bgr(hsv const& in);

    struct hsv
    {
      hsv()
      {}

      hsv(uint8_t h, uint8_t s, uint8_t v)
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

      uint8_t operator[](unsigned idx)
      {
        switch (idx)
        {
        case 0:
          return h;
          break;
        case 1:
          return s;
          break;
        case 2:
          return v;
          break;
        default:
          return 0;
        }
      }

      uint8_t h;
      uint8_t s;
      uint8_t v;
    };

    struct hsvRange
    {
      uint8_t hMin;
      uint8_t hMax;
      uint8_t sMin;
      uint8_t sMax;
      uint8_t vMin;
      uint8_t vMax;

      hsvRange();

      hsvRange(
        uint8_t hMin, uint8_t hMax,
        uint8_t sMin, uint8_t sMax,
        uint8_t vMin, uint8_t vMax);

      uint8_t getHMid() const;
      uint8_t getSMid() const;
      uint8_t getVMid() const;

      Colour::hsv toHsv() const;

      /** Obtain the center colour for the range, in BGR format. */
      Colour::bgr toBgr() const;

      bool contains(hsv const& hsv) const;

      bool isValid() const;

      bool operator==(hsvRange const& other) const
      {
        return
          hMin == other.hMin &&
          hMax == other.hMax &&
          sMin == other.sMin &&
          sMax == other.sMax &&
          vMin == other.vMin &&
          vMax == other.vMax;
      }

      static hsvRange fromBytes(uint8_t hMin, uint8_t hMax, uint8_t sMin, uint8_t sMax, uint8_t vMin, uint8_t vMax);
      static hsvRange fromDoubles(double hMin, double hMax, double sMin, double sMax, double vMin, double vMax);

      hsvRange withHMin(uint8_t value) const { return hsvRange(value, hMax, sMin, sMax, vMin, vMax); }
      hsvRange withHMax(uint8_t value) const { return hsvRange(hMin, value, sMin, sMax, vMin, vMax); }
      hsvRange withSMin(uint8_t value) const { return hsvRange(hMin, hMax, value, sMax, vMin, vMax); }
      hsvRange withSMax(uint8_t value) const { return hsvRange(hMin, hMax, sMin, value, vMin, vMax); }
      hsvRange withVMin(uint8_t value) const { return hsvRange(hMin, hMax, sMin, sMax, value, vMax); }
      hsvRange withVMax(uint8_t value) const { return hsvRange(hMin, hMax, sMin, sMax, vMin, value); }
    };

    inline void yCbCrToBgrInPlace(uint8_t* pxl)
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
