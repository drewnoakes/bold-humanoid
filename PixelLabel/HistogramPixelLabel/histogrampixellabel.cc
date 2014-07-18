#include "histogrampixellabel.hh"

using namespace bold;
using namespace std;

template<uint8_t CHANNEL_BITS>
HistogramPixelLabel<CHANNEL_BITS>::HistogramPixelLabel(std::string name, LabelClass id)
  : PixelLabel{name, id},
  d_totalCount{0}
{

  static_assert(CHANNEL_BITS > 0 && CHANNEL_BITS <= 8,
                "Channel bits must be larger than 0 and less than or equal to 8 bits");
  d_histogram.fill(0);
}

template<uint8_t CHANNEL_BITS>
void HistogramPixelLabel<CHANNEL_BITS>::addSample(Colour::hsv const& pixelColour)
{
  auto index = hsvToIndex(pixelColour);
  ++d_histogram[index];
  ++d_totalCount;
}

template<uint8_t CHANNEL_BITS>
unsigned HistogramPixelLabel<CHANNEL_BITS>::getTotalCount() const
{
  return d_totalCount;
}

template<uint8_t CHANNEL_BITS>
float HistogramPixelLabel<CHANNEL_BITS>::labelProb(Colour::hsv const& pixelColour) const
{
  return
    d_totalCount == 0 ?
    1.0 / (TOTAL_BINS) :
    float(getHistogramValue(pixelColour.h, pixelColour.s, pixelColour.v)) / d_totalCount;
}

template<uint8_t CHANNEL_BITS>
Colour::hsv HistogramPixelLabel<CHANNEL_BITS>::modalColour() const
{
  auto modalBinIndex = std::distance(begin(d_histogram), std::max_element(begin(d_histogram), end(d_histogram)));
  return indexToHsv(modalBinIndex);
}

template<uint8_t CHANNEL_BITS>
unsigned HistogramPixelLabel<CHANNEL_BITS>::getHistogramValue(uint8_t h, uint8_t s, uint8_t v) const
{
  return d_histogram.at(hsvToIndex(h, s, v));
}

template<uint8_t CHANNEL_BITS>
unsigned HistogramPixelLabel<CHANNEL_BITS>::hsvToIndex(Colour::hsv const& hsv)
{
  return hsvToIndex(hsv.h, hsv.s, hsv.v);
}

template<uint8_t CHANNEL_BITS>
unsigned HistogramPixelLabel<CHANNEL_BITS>::hsvToIndex(uint8_t h, uint8_t s, uint8_t v)
{
  return
    ((h >> (8 - CHANNEL_BITS)) << (2 * CHANNEL_BITS)) +
    ((s >> (8 - CHANNEL_BITS)) << CHANNEL_BITS) +
    (v >> (8 - CHANNEL_BITS));
}

template<uint8_t CHANNEL_BITS>
Colour::hsv HistogramPixelLabel<CHANNEL_BITS>::indexToHsv(unsigned idx)
{
  uint8_t vIdx = idx & (NBINS - 1);
  idx >>= CHANNEL_BITS;
  uint8_t sIdx = idx & (NBINS - 1);
  idx >>= CHANNEL_BITS;
  uint8_t hIdx = idx;
      
  return Colour::hsv(hIdx << (8 - CHANNEL_BITS), sIdx << (8 - CHANNEL_BITS), vIdx << (8 - CHANNEL_BITS));
}

template<uint8_t CHANNEL_BITS>
uint8_t HistogramPixelLabel<CHANNEL_BITS>::channelIndexToValue(uint8_t idx)
{
  return idx << (8 - CHANNEL_BITS);
}

template<uint8_t CHANNEL_BITS>
uint8_t HistogramPixelLabel<CHANNEL_BITS>::valueToChannelIndex(uint8_t val)
{
  return val >> (8 - CHANNEL_BITS);
}

template<uint8_t CHANNEL_BITS>
unsigned HistogramPixelLabel<CHANNEL_BITS>::getBinSize()
{
  return BIN_SIZE;
}

template<uint8_t CHANNEL_BITS>
unsigned HistogramPixelLabel<CHANNEL_BITS>::getNBins()
{
  return NBINS;
}

template<uint8_t CHANNEL_BITS>
cv::Mat HistogramPixelLabel<CHANNEL_BITS>::getHSImage() const
{
  cv::Mat hueImg = cv::Mat::zeros(NBINS, NBINS, CV_8UC1);
  if (d_totalCount == 0)
    return hueImg;
  
  cv::Mat satImg = cv::Mat::zeros(NBINS, NBINS, CV_8UC1);
  cv::Mat valueImg = cv::Mat::zeros(NBINS, NBINS, CV_8UC1);
  cv::Mat alphaImg = cv::Mat::zeros(NBINS, NBINS, CV_8UC1);
  cv::Mat accumImg = cv::Mat::zeros(NBINS, NBINS, CV_32FC1);
  float max = 0.0f;
  for (unsigned idx = 0; idx < TOTAL_BINS; ++idx)
  {
    auto hsvColour = indexToHsv(idx);
    auto hIdx = valueToChannelIndex(hsvColour.h);
    auto sIdx = valueToChannelIndex(hsvColour.s);
    auto v = accumImg.at<float>(hIdx, sIdx) + d_histogram[idx];
    if (v > max)
      max = v;
    accumImg.at<float>(hIdx, sIdx) = v;
  }
      
  for (unsigned hIdx = 0; hIdx < NBINS; ++hIdx)
    for (unsigned sIdx = 0; sIdx < NBINS; ++sIdx)
    {
      uint8_t a = 255 * accumImg.at<float>(hIdx, sIdx) / max;
      valueImg.at<uint8_t>(hIdx, sIdx) = 128;
      hueImg.at<uint8_t>(hIdx, sIdx) = 180 * (hIdx + 0.5f) / NBINS;
      satImg.at<uint8_t>(hIdx, sIdx) = 255 * (sIdx + 0.5f) / NBINS;
      alphaImg.at<uint8_t>(hIdx, sIdx) = a;
    }

  std::cout << "max: " << max << " " << d_totalCount << std::endl;

  cv::Mat result;
  auto channels = std::vector<cv::Mat>{hueImg, satImg, valueImg};
  cv::merge(channels, result);
  cv::cvtColor(result, result, CV_HSV2BGR);
  cv::split(result, channels);
  channels.push_back(alphaImg);
  cv::merge(channels, result);
  return result;
}

template<uint8_t CHANNEL_BITS>
cv::Mat HistogramPixelLabel<CHANNEL_BITS>::getHVImage() const
{
  cv::Mat hueImg = cv::Mat::zeros(NBINS, NBINS, CV_8UC1);
  if (d_totalCount == 0)
    return hueImg;

  cv::Mat satImg = cv::Mat::zeros(NBINS, NBINS, CV_8UC1);
  cv::Mat valueImg = cv::Mat::zeros(NBINS, NBINS, CV_8UC1);
  cv::Mat alphaImg = cv::Mat::zeros(NBINS, NBINS, CV_8UC1);
  cv::Mat accumImg = cv::Mat::zeros(NBINS, NBINS, CV_32FC1);
  float max = 0.0f;
  for (unsigned idx = 0; idx < TOTAL_BINS; ++idx)
  {
    auto hsvColour = indexToHsv(idx);
    auto hIdx = valueToChannelIndex(hsvColour.h);
    auto vIdx = valueToChannelIndex(hsvColour.v);
    auto v = accumImg.at<float>(hIdx, vIdx) + d_histogram[idx];
    if (v > max)
      max = v;
    accumImg.at<float>(hIdx, vIdx) = v;
  }
      
  for (unsigned hIdx = 0; hIdx < NBINS; ++hIdx)
    for (unsigned vIdx = 0; vIdx < NBINS; ++vIdx)
    {
      uint8_t a = 255 * accumImg.at<float>(hIdx, vIdx) / max;
      satImg.at<uint8_t>(hIdx, vIdx) = 255;
      hueImg.at<uint8_t>(hIdx, vIdx) = 180 * (hIdx + 0.5f) / NBINS;
      valueImg.at<uint8_t>(hIdx, vIdx) = 255 * (vIdx + 0.5f) / NBINS;
      alphaImg.at<uint8_t>(hIdx, vIdx) = a;
    }

  std::cout << "max: " << max << " " << d_totalCount << std::endl;

  cv::Mat result;
  auto channels = std::vector<cv::Mat>{hueImg, satImg, valueImg};
  cv::merge(channels, result);
  cv::cvtColor(result, result, CV_HSV2BGR);
  cv::split(result, channels);
  channels.push_back(alphaImg);
  cv::merge(channels, result);
  return result;
}

template<uint8_t CHANNEL_BITS>
cv::Mat HistogramPixelLabel<CHANNEL_BITS>::getSVImage() const
{
  cv::Mat hueImg = cv::Mat::zeros(NBINS, NBINS, CV_8UC1);
  if (d_totalCount == 0)
    return hueImg;

  cv::Mat satImg = cv::Mat::zeros(NBINS, NBINS, CV_8UC1);
  cv::Mat valueImg = cv::Mat::zeros(NBINS, NBINS, CV_8UC1);
  cv::Mat alphaImg = cv::Mat::zeros(NBINS, NBINS, CV_8UC1);
  cv::Mat accumImg = cv::Mat::zeros(NBINS, NBINS, CV_32FC1);
  float max = 0.0f;
  for (unsigned idx = 0; idx < TOTAL_BINS; ++idx)
  {
    auto hsvColour = indexToHsv(idx);
    auto sIdx = valueToChannelIndex(hsvColour.s);
    auto vIdx = valueToChannelIndex(hsvColour.v);
    auto v = accumImg.at<float>(sIdx, vIdx) + d_histogram[idx];
    if (v > max)
      max = v;
    accumImg.at<float>(sIdx, vIdx) = v;
  }
      
  for (unsigned sIdx = 0; sIdx < NBINS; ++sIdx)
    for (unsigned vIdx = 0; vIdx < NBINS; ++vIdx)
    {
      uint8_t a = 255 * accumImg.at<float>(sIdx, vIdx) / max;
      satImg.at<uint8_t>(sIdx, vIdx) = 255 * (sIdx + 0.5f) / NBINS;
      hueImg.at<uint8_t>(sIdx, vIdx) = 0;
      valueImg.at<uint8_t>(sIdx, vIdx) = 255 * (vIdx + 0.5f) / NBINS;
      alphaImg.at<uint8_t>(sIdx, vIdx) = a;
    }

  std::cout << "max: " << max << " " << d_totalCount << std::endl;

  cv::Mat result;
  auto channels = std::vector<cv::Mat>{hueImg, satImg, valueImg};
  cv::merge(channels, result);
  cv::cvtColor(result, result, CV_HSV2BGR);
  cv::split(result, channels);
  channels.push_back(alphaImg);
  cv::merge(channels, result);
  return result;
}


template<uint8_t CHANNEL_BITS>
void HistogramPixelLabel<CHANNEL_BITS>::write(ostream& out) const
{
  out.write(reinterpret_cast<char const*>(d_histogram.data()), d_histogram.size() * sizeof(unsigned));
}

template<uint8_t CHANNEL_BITS>
void HistogramPixelLabel<CHANNEL_BITS>::read(istream& in)
{
  in.read(reinterpret_cast<char*>(d_histogram.data()), d_histogram.size() * sizeof(unsigned));
  d_totalCount = 0;
  for (auto v : d_histogram)
    d_totalCount += v;
}

template class bold::HistogramPixelLabel<8>;
template class bold::HistogramPixelLabel<7>;
template class bold::HistogramPixelLabel<6>;
template class bold::HistogramPixelLabel<5>;
template class bold::HistogramPixelLabel<4>;
template class bold::HistogramPixelLabel<3>;
template class bold::HistogramPixelLabel<2>;
template class bold::HistogramPixelLabel<1>;
