#ifndef BOLD_LUTBUILDER_HH
#define BOLD_LUTBUILDER_HH

#include <vector>

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
