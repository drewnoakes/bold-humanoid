#pragma once 

#include "../pixellabel.hh"

namespace bold
{
  template<uint8_t CHANNEL_BITS>
  class HistogramPixelLabel : public PixelLabel
  {
  public:
    HistogramPixelLabel(std::string name)
      : PixelLabel{name},
        d_totalCount{0}
    {
      static_assert(CHANNEL_BITS > 0 && CHANNEL_BITS <= 8, "Channel bits must be larthe than 0 and less than or equal to 8 bits");
    }

    float labelProb(Colour::hsv const& pixelColour) const override
    {
      return
        d_totalCount == 0 ?
        1.0 / (NBINS * NBINS * NBINS) :
        float(getHistogramValue(pixelColour.h, pixelColour.s, pixelColour.v)) / d_totalCount;
    }

    Colour::hsv modalColour() const override
    {
      auto modalBinIndex = std::distance(begin(d_histogram), std::max_element(begin(d_histogram), end(d_histogram)));
      return indexToHsv(modalBinIndex);
    }

    unsigned getHistogramValue(uint8_t h, uint8_t s, uint8_t v) const
    {
      return d_histogram.at(hsvToIndex(h, s, v));
    }

    constexpr static unsigned hsvToIndex(uint8_t h, uint8_t s, uint8_t v)
    {
      return (h << (2 * CHANNEL_BITS)) + (s << CHANNEL_BITS) + v;
    }

    static Colour::hsv indexToHsv(unsigned idx)
    {
      uint8_t v = idx & (NBINS - 1);
      idx >>= CHANNEL_BITS;
      uint8_t s = idx & (NBINS - 1);
      idx >>= CHANNEL_BITS;
      uint8_t h = idx;
      
      return Colour::hsv(h, s, v);
    }

    static unsigned getBinSize()
    {
      return BIN_SIZE;
    }

    static unsigned getNBins()
    {
      return NBINS;
    }

  private:
    constexpr static unsigned BIN_SIZE = (256 >> CHANNEL_BITS);
    constexpr static unsigned NBINS = 1 << CHANNEL_BITS;

    std::array<unsigned, NBINS * NBINS * NBINS> d_histogram;
    unsigned d_totalCount;
  };
}
