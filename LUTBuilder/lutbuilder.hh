#ifndef BOLD_LUTBUILDER_HH
#define BOLD_LUTBUILDER_HH

#include <vector>

#include "../Colour/colour.hh"
#include "../PixelLabel/pixellabel.hh"

namespace bold
{
  class LUTBuilder
  {
  public:
    static uchar* buildBGR24FromHSVRanges(std::vector<bold::PixelLabel> const& labels);
    static uchar* buildBGR18FromHSVRanges(std::vector<bold::PixelLabel> const& labels);

    /**
     * Returns the id of the first label that matches the specified BGR colour.
     */
    static uchar labelPixel(std::vector<bold::PixelLabel> const& labels, Colour::bgr const& bgr);
  };
}

#endif
