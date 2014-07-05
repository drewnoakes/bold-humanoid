#pragma once 

#include "../pixellabel.hh"
#include <opencv2/imgproc/imgproc.hpp>

namespace bold
{
  template<uint8_t CHANNEL_BITS>
  class HistogramPixelLabel : public PixelLabel
  {
  public:
    HistogramPixelLabel(std::string name);

    void addSample(Colour::hsv const& pixelColour);

    unsigned getTotalCount() const;

    float labelProb(Colour::hsv const& pixelColour) const override;

    Colour::hsv modalColour() const override;

    unsigned getHistogramValue(uint8_t h, uint8_t s, uint8_t v) const;

    static unsigned hsvToIndex(Colour::hsv const& hsv);

    static unsigned hsvToIndex(uint8_t h, uint8_t s, uint8_t v);

    static Colour::hsv indexToHsv(unsigned idx);

    static uint8_t channelIndexToValue(uint8_t idx);

    static uint8_t valueToChannelIndex(uint8_t val);

    static unsigned getBinSize();

    static unsigned getNBins();

    cv::Mat getHSImage() const;
    cv::Mat getHVImage() const;
    cv::Mat getSVImage() const;

  private:
    constexpr static unsigned BIN_SIZE = (256 >> CHANNEL_BITS);
    constexpr static unsigned NBINS = 1 << CHANNEL_BITS;
    constexpr static unsigned TOTAL_BINS = NBINS * NBINS * NBINS;

    std::array<unsigned, TOTAL_BINS> d_histogram;
    unsigned d_totalCount;
  };

  extern template class bold::HistogramPixelLabel<8>;
  extern template class bold::HistogramPixelLabel<7>;
  extern template class bold::HistogramPixelLabel<6>;
  extern template class bold::HistogramPixelLabel<5>;
  extern template class bold::HistogramPixelLabel<4>;
  extern template class bold::HistogramPixelLabel<3>;
  extern template class bold::HistogramPixelLabel<2>;
  extern template class bold::HistogramPixelLabel<1>;
}
