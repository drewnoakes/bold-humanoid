#ifndef BOLD_LUTBUILDER_HH
#define BOLD_LUTBUILDER_HH

#include <vector>

#include "../Colour/colour.hh"

namespace bold
{
  class LUTBuilder
  {
  public:
    unsigned char* buildBGR24FromHSVRanges(std::vector<Colour::hsvRange> const& ranges);
    unsigned char* buildBGR18FromHSVRanges(std::vector<Colour::hsvRange> const& ranges);
  };
}

#endif
