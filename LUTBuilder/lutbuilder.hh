#ifndef BOLD_LUTBUILDER_HH
#define BOLD_LUTBUILDER_HH

#include <vector>

#include <LinuxDARwIn.h>

namespace bold
{
  struct bgr
  {
    bgr(int _b, int _g, int _r)
      : b(_b), g(_g), r(_r)
    {}

    unsigned char b;
    unsigned char g;
    unsigned char r;
  };

  struct hsv
  {
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

  class LUTBuilder
  {
  public:
    unsigned char* buildBGR24FromHSVRanges(std::vector<hsvRange> const& ranges);
    unsigned char* buildBGR18FromHSVRanges(std::vector<hsvRange> const& ranges);

  private:

    hsv bgr2hsv(bgr const& in);
  };
}

#endif
