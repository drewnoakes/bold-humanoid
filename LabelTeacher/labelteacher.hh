#pragma once

#include "../PixelFilterChain/pixelfilterchain.hh"
#include "../PixelLabel/pixellabel.hh"
#include "../Setting/setting.hh"

#include <vector>
#include <string>
#include <memory>

namespace bold
{
  template<uint8_t CHANNEL_BITS>
  class LabelTeacher
  {
  public:
    LabelTeacher(std::vector<std::shared_ptr<PixelLabel>> labels);
    
    std::vector<std::shared_ptr<PixelLabel>> getLabels() const;

    void train(unsigned labelIdx, cv::Mat const& mask);

    cv::Mat label(unsigned labelIdx) const;

    bool requestedSnapShot() const { return d_snapshotRequested; }
    bool requestedLabel() const {return d_labelRequested; }

    void setYUVTrainImage(cv::Mat yuvTrainImg);
    cv::Mat getYUVTrainImage() const { return d_yuvTrainImage; }
    cv::Mat getBGRTrainImage() const
    {
      if (d_yuvTrainImage.rows == 0)
        return d_yuvTrainImage;

      cv::Mat bgrImage = d_yuvTrainImage.clone();

      PixelFilterChain chain;
      chain.pushFilter(&Colour::yCbCrToBgrInPlace);
      chain.applyFilters(bgrImage);
      return bgrImage;
    }

    cv::Mat getMask() const { return d_mask; }

    void setSeedPoint(Eigen::Vector2i point);

    cv::Mat floodFill() const;

    unsigned getLabelToTrain() const { return d_labelToTrain; }

  private:
    std::vector<std::shared_ptr<PixelLabel>> d_labels;
    cv::Mat d_yuvTrainImage;
    cv::Mat d_mask;

    Eigen::Vector2i d_seedPoint;
    float d_maxFloodDiff;
    unsigned d_labelToTrain;

    bool d_snapshotRequested;
    bool d_labelRequested;
    bool d_fixedRange;

  };
  
  //////

  template<uint8_t CHANNEL_BITS>
  LabelTeacher<CHANNEL_BITS>::LabelTeacher(std::vector<std::shared_ptr<PixelLabel>> labels)
    : d_labels{labels},
    d_yuvTrainImage{},
    d_seedPoint{0,0},
    d_snapshotRequested{false},
    d_labelRequested{false},
    d_fixedRange{false}
  {
  }

  template<uint8_t CHANNEL_BITS>
  std::vector<std::shared_ptr<PixelLabel>> LabelTeacher<CHANNEL_BITS>::getLabels() const
  {
    return d_labels;
  }

  template<uint8_t CHANNEL_BITS>
  void LabelTeacher<CHANNEL_BITS>::setYUVTrainImage(cv::Mat yuvTrainImage)
  {
    d_yuvTrainImage = yuvTrainImage;
    d_snapshotRequested = false;
  }

  template<uint8_t CHANNEL_BITS>
  void LabelTeacher<CHANNEL_BITS>::setSeedPoint(Eigen::Vector2i point)
  {
    d_seedPoint = point;
  }

  template<uint8_t CHANNEL_BITS>
  inline cv::Mat LabelTeacher<CHANNEL_BITS>::floodFill() const
  {
    cv::Mat mask = cv::Mat::zeros(d_yuvTrainImage.rows + 2, d_yuvTrainImage.cols + 2, CV_8UC1);

    auto mfd = cv::Scalar(d_maxFloodDiff, d_maxFloodDiff, d_maxFloodDiff);
    cv::floodFill(d_yuvTrainImage, mask,
                  cv::Point{d_seedPoint.x(), d_seedPoint.y()}, cv::Scalar(255), 0,
                  mfd, mfd, 4 | (d_fixedRange ? cv::FLOODFILL_FIXED_RANGE : 0) | cv::FLOODFILL_MASK_ONLY | (255 << 8));

    return cv::Mat{mask, cv::Rect(1, 1, d_yuvTrainImage.cols, d_yuvTrainImage.rows)};
    
    return mask;
  }
  
  template<uint8_t CHANNEL_BITS>
  void LabelTeacher<CHANNEL_BITS>::train(unsigned labelIdx, cv::Mat const& mask)
  {
    ASSERT(mask.type() == CV_8UC1);
    ASSERT(d_yuvTrainImage.cols == mask.cols && d_yuvTrainImage.rows == mask.rows);

    for (unsigned i = 0; i < mask.rows; ++i)
    {
      uint8_t const* trainImageRow = d_yuvTrainImage.ptr<uint8_t>(i);
      uint8_t const* maskRow = mask.ptr<uint8_t>(i);

      for (unsigned j = 0; j < mask.cols; ++j)
        if (maskRow[j] != 0)
        {
          auto yuv = Colour::YCbCr{trainImageRow[j * 3 + 0], trainImageRow[j * 3 + 1], trainImageRow[j * 3 + 2]};
          auto bgr = yuv.toBgrInt();
          auto hsv = bgr2hsv(bgr);

          d_labels[labelIdx]->addSample(hsv);
        }
    }
  }

  template<uint8_t CHANNEL_BITS>
  cv::Mat LabelTeacher<CHANNEL_BITS>::label(unsigned labelIdx) const
  {
    auto labelImg = cv::Mat{d_yuvTrainImage.rows, d_yuvTrainImage.cols, CV_8UC1};
    for (unsigned i = 0; i < labelImg.rows; ++i)
    {
      uint8_t const* trainImageRow = d_yuvTrainImage.ptr<uint8_t>(i);
      uint8_t* labelRow = labelImg.ptr<uint8_t>(i);

      for (unsigned j = 0; j < labelImg.cols; ++j)
      {
        auto yuv = Colour::YCbCr{trainImageRow[j * 3 + 0], trainImageRow[j * 3 + 1], trainImageRow[j * 3 + 2]};
        auto bgr = yuv.toBgrInt();
        auto hsv = bgr2hsv(bgr);
        labelRow[j] = d_labels[labelIdx]->labelProb(hsv) * 255;
      }
    }

    cv::normalize(labelImg, labelImg, 0, 255, cv::NORM_MINMAX);
    cv::Mat zero = cv::Mat::zeros(labelImg.rows, labelImg.cols, CV_8UC1);
    cv::Mat _colourLabelImage;
    cv::merge(std::vector<cv::Mat>{zero, labelImg, zero}, _colourLabelImage);
    return _colourLabelImage;
  }
}
