#ifndef BOLD_HOUGHLINEEXTRACTOR_HH
#define BOLD_HOUGHLINEEXTRACTOR_HH

#include "../Geometry/geometry.hh"
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
    std::vector<bold::Line> findLines(bold::HoughLineAccumulator& accumulator, int threshold = 10, double angleSearch = (5.0/180)*M_PI, int radiusSearch = 10);

    /**
     * Finds the single most probable line in the accumulator.
     */
    bold::Line findMaxLine(bold::HoughLineAccumulator& accumulator);
  };
}

#endif