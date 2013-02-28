#ifndef BOLD_COLOUR_HH
#define BOLD_COLOUR_HH

#include <vector>
#include <string>

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

      bgr(int _b, int _g, int _r)
        : b(_b), g(_g), r(_r)
      {}

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
      : h(hue), hRange(hueRange),
        s(saturation), sRange(saturationRange),
        v(value), vRange(valueRange)
      {};

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
}

#endif
