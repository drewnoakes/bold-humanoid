#ifndef BOLD_HOUGHLINEEXTRACTOR_HH
#define BOLD_HOUGHLINEEXTRACTOR_HH

#include "../HoughLine/houghline.hh"
#include "../HoughLineAccumulator/houghlineaccumulator.hh"

#include <vector>

namespace bold
{
  class HoughLineExtractor
  {
  public:
    HoughLineExtractor() {}

    /**
     * Searches for lines in a {@link HoughLineAccumulator}.
     */
    std::vector<bold::HoughLine> findLines(bold::HoughLineAccumulator& accumulator);
  };
}

#endif