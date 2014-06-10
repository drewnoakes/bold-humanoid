#pragma once

#include <vector>
#include <memory>

#include "../Colour/colour.hh"
#include "../PixelLabel/pixellabel.hh"

namespace bold
{
  class LUTBuilder
  {
  public:
    static std::shared_ptr<uchar const> buildLookUpTableBGR24(std::vector<std::shared_ptr<PixelLabel>> const& labels);
    static std::shared_ptr<uchar const> buildLookUpTableBGR18(std::vector<std::shared_ptr<PixelLabel>> const& labels);
    static std::shared_ptr<uchar const> buildLookUpTableYCbCr18(std::vector<std::shared_ptr<PixelLabel>> const& labels);

    /**
     * Returns the id of the first label that matches the specified BGR colour.
     */
    static uchar labelPixel(std::vector<std::shared_ptr<PixelLabel>> const& labels, Colour::bgr const& bgr);
  };
}
