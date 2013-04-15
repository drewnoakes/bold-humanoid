#ifndef BOLD_HOUGHLINEEXTRACTOR_HH
#define BOLD_HOUGHLINEEXTRACTOR_HH

#include "../geometry/Line.hh"
#include "../HoughLineAccumulator/houghlineaccumulator.hh"
#include "../util/Candidate.hh"

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
    std::vector<Candidate<Line>> findLines(HoughLineAccumulator& accumulator, int threshold = 10, double angleSearch = (5.0/180)*M_PI, int radiusSearch = 10) const;

    /**
     * Finds the single most probable line in the accumulator.
     */
    Line findMaxLine(HoughLineAccumulator& accumulator) const;
  };
}

#endif