#pragma once

#include "../PixelLabel/HistogramPixelLabel/histogrampixellabel.hh"
#include <vector>
#include <string>
#include <memory>

namespace bold
{
  class HistogramLabelTeacherBase
  {
  public:
    HistogramLabelTeacherBase();

    bool requestedSnapShot() const { return d_snapshotRequested; }

    void setTrainImage(cv::Mat trainImg);
    cv::Mat getTrainImage() const { return d_trainImage; }

    void setSeedPoint(Eigen::Vector2i point);

    void setMaxDiff(uint8_t maxDiff);

    cv::Mat floodFill() const;

  protected:
    cv::Mat d_trainImage;
    Eigen::Vector2i d_seedPoint;
    uint8_t d_maxDiff;

  private:
    bool d_snapshotRequested;
  };

  template<uint8_t CHANNEL_BITS>
  class HistogramLabelTeacher : public HistogramLabelTeacherBase
  {
  public:
    HistogramLabelTeacher(std::vector<std::string> const& names);
    
    std::vector<HistogramPixelLabel<CHANNEL_BITS>> getLabels() const;

    void train(std::string const& labelName, cv::Mat const& mask);

  private:
    std::vector<HistogramPixelLabel<CHANNEL_BITS>> d_labels;
  };
  


  template<uint8_t CHANNEL_BITS>
  HistogramLabelTeacher<CHANNEL_BITS>::HistogramLabelTeacher(std::vector<std::string> const& names)
    : d_labels{}
  {
    for (auto const& name : names)
      d_labels.emplace_back(name);
  }

  template<uint8_t CHANNEL_BITS>
  std::vector<HistogramPixelLabel<CHANNEL_BITS>> HistogramLabelTeacher<CHANNEL_BITS>::getLabels() const
  {
    return d_labels;
  }

  inline void HistogramLabelTeacherBase::setTrainImage(cv::Mat trainImage)
  {
    d_trainImage = trainImage;
    d_snapshotRequested = false;
  }

  inline void HistogramLabelTeacherBase::setSeedPoint(Eigen::Vector2i point)
  {
    d_seedPoint = point;
  }

  inline void HistogramLabelTeacherBase::setMaxDiff(uint8_t maxDiff)
  {
    d_maxDiff = maxDiff;
  }

  inline cv::Mat HistogramLabelTeacherBase::floodFill() const
  {
    cv::Mat mask = cv::Mat::zeros(d_trainImage.rows + 2, d_trainImage.cols + 2, CV_8UC1);

    cv::floodFill(d_trainImage, mask,
                  cv::Point{d_seedPoint.x(), d_seedPoint.y()}, cv::Scalar(255), 0,
                  d_maxDiff, d_maxDiff, 4 | cv::FLOODFILL_MASK_ONLY | (255 << 8));
    
    return cv::Mat{mask, cv::Rect(1, 1, d_trainImage.cols, d_trainImage.rows)};
    
    return mask;
  }
  
  template<uint8_t CHANNEL_BITS>
  void HistogramLabelTeacher<CHANNEL_BITS>::train(std::string const& name, cv::Mat const& mask)
  {
    ASSERT(mask.type() == CV_8UC1);
    ASSERT(d_trainImage.cols == mask.cols && d_trainImage.rows == mask.rows);

    auto labelIter = std::find_if(begin(d_labels), end(d_labels), [&name](HistogramPixelLabel<CHANNEL_BITS> const& label) {
        return label.getName() == name;
      });

    if (labelIter == end(d_labels))
      return;

    for (unsigned i = 0; i < mask.rows; ++i)
    {
      uint8_t const* trainImageRow = d_trainImage.ptr<uint8_t>(i);
      uint8_t const* maskRow = mask.ptr<uint8_t>(i);

      for (unsigned j = 0; j < mask.cols; ++j)
        if (maskRow[j] != 0)
        {
          auto hsv = Colour::hsv{trainImageRow[0], trainImageRow[1], trainImageRow[2]};
          labelIter->addSample(hsv);
        }
    }
  }


}
