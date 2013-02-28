#ifndef BOLD_HOUGHLINEEXTRACTOR_HH
#define BOLD_HOUGHLINEEXTRACTOR_HH

#include "../HoughLine/houghline.hh"
#include "../HoughLineAccumulator/houghlineaccumulator.hh"

#include <vector>
#include <cmath>

namespace bold
{
  class HoughLineExtractor
  {
  public:
    HoughLineExtractor() {}

    /**
     * Searches for lines in a {@link HoughLineAccumulator}.
     */
    std::vector<bold::HoughLine> findLines(bold::HoughLineAccumulator& accumulator, int threshold = 10, double angleSearch = (5.0/180)*M_PI, int radiusSearch = 10);

    /**
     * Finds the single most probable line in the accumulator.
     */
    bold::HoughLine findMaxLine(bold::HoughLineAccumulator& accumulator);
  };
}

#endif