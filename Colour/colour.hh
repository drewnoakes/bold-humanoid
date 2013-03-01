#ifndef BOLD_COLOUR_HH
#define BOLD_COLOUR_HH

//#include <cmath>
#include <vector>
#include <string>
#include <opencv2/core/core.hpp>

// Forwards declaration, as this type is defined in the robotis repository (yuck, I know)
class minIni
{
public:
  int geti(const std::string& Section, const std::string& Key, int DefValue=0) const;
};

namespace bold
{
  class Colour
  {
  public:
    struct bgr
    {
      bgr()
      {}

      bgr(int b, int g, int r)
        : b(b), g(g), r(r)
      {}

      bgr invert()
      {
        return bgr(255-b, 255-g, 255-r);
      }

      cv::Scalar toScalar()
      {
        return cv::Scalar(b, g, r);
      }

      unsigned char b;
      unsigned char g;
      unsigned char r;
    };

    struct hsv
    {
      hsv()
      {}

      hsv(int h, int s, int v)
        : h(h), s(s), v(v)
      {}

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

      hsvRange() {};

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

      inline bool contains(hsv hsv)
      {
        int hDiff = abs(hsv.h - h);
        hDiff = std::min(hDiff, 192 - hDiff);

        return (hDiff <= hRange &&
                hsv.s >= s - sRange && hsv.s <= s + sRange &&
                hsv.v >= v - vRange && hsv.v <= v + vRange);
      }

      static hsvRange fromConfig(
        minIni const& ini,
        std::string objectName,
        int hue,        int hueRange,
        int saturation, int saturationRange,
        int value,      int valueRange
      )
      {
        hsvRange range;
        range.h      = ini.geti("Vision", objectName + "Hue",             hue);
        range.hRange = ini.geti("Vision", objectName + "HueRange",        hueRange);
        range.s      = ini.geti("Vision", objectName + "Saturation",      saturation);
        range.sRange = ini.geti("Vision", objectName + "SaturationRange", saturationRange);
        range.v      = ini.geti("Vision", objectName + "Value",           value);
        range.vRange = ini.geti("Vision", objectName + "ValueRange",      valueRange);
        return range;
      }
    };

    static hsv bgr2hsv(bgr const& in);
    static bgr hsv2bgr(hsv const& in);
  };

  std::ostream& operator<<(std::ostream &stream, Colour::hsv const& hsv);
  std::ostream& operator<<(std::ostream &stream, Colour::hsvRange const& hsv);
}

#endif
