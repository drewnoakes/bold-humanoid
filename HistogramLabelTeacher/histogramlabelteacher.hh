#pragma once

#include "../PixelLabel/HistogramPixelLabel/histogrampixellabel.hh"

#include <vector>
#include <string>

namespace bold {
  
  template<uint8_t CHANNEL_BITS>
  class HistogramLabelTeacher
  {
  public:
    HistogramLabelTeacher(std::vector<std::string> const& names);
    
    std::vector<HistogramPixelLabel<CHANNEL_BITS>> getLabels() const;

  private:
    std::vector<HistogramPixelLabel<CHANNEL_BITS>> d_labels;
  };
  
  template<uint8_t CHANNEL_BITS>
  HistogramLabelTeacher<CHANNEL_BITS>::HistogramLabelTeacher(std::vector<std::string> const& names)
  {
  }

  template<uint8_t CHANNEL_BITS>
  std::vector<HistogramPixelLabel<CHANNEL_BITS>> HistogramLabelTeacher<CHANNEL_BITS>::getLabels() const
  {
    return d_labels;
  }
}
