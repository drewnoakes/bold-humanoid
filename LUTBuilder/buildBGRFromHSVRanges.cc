#include "lutbuilder.ih"

unsigned char* LUTBuilder::buildBGRFromHSVRanges(std::vector<hsvRange> const& ranges)
{
  unsigned char* LUT = new unsigned char[1<<24];
  unsigned char* p = LUT;
  for (int b = 0; b < 256; ++b)
    for (int g = 0; g < 256; ++g)
      for (int r = 0; r < 256; ++r)
        {
          hsv hsv = bgr2hsv(bgr(b, g, r));

          *p = 0;

          // test h
          for (unsigned i = 0; i < ranges.size(); ++i)
          {
            hsvRange range = ranges[i];

            int diff = abs((int)hsv.h - range.h);
            diff = min(diff, 192 - diff);

            if (diff <= range.hRange &&
                hsv.s >= range.s - range.sRange && hsv.s <= range.s + range.sRange &&
                hsv.v >= range.v - range.vRange && hsv.v <= range.v + range.vRange)
              *p = i+1;
          }
          ++p;
        }

  return LUT;
}

