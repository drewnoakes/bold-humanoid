#pragma once

#include "../PixelFilterChain/pixelfilterchain.hh"
#include "../PixelLabel/pixellabel.hh"
#include "../PixelLabel/RangePixelLabel/rangepixellabel.hh"
#include "../Setting/setting.hh"
#include "../Setting/setting-implementations.hh"
#include "../Config/config.hh"
#include "../util/json.hh"

#include <vector>
#include <string>
#include <memory>

namespace bold
{
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

  inline LabelTeacher::LabelTeacher(std::vector<std::shared_ptr<PixelLabel>> labels)
    : d_labels{labels},
    d_yuvTrainImage{},
    d_seedPoint{0,0},
    d_snapshotRequested{false},
    d_labelRequested{false},
    d_fixedRange{false}
  {
    Config::getSetting<int>("histogram-label-teacher.max-flood-diff")->track([this](int val) {
        d_maxFloodDiff = val;
        log::info("HistogramLabelTeacherBase") << "Setting maxFloodDiff: " << d_maxFloodDiff;
      });

    std::map<int, std::string> enumOptions;
    for (unsigned i = 0; i < labels.size(); ++i)
      enumOptions[i] = labels[i]->getName();

    auto setting = new EnumSetting("histogram-label-teacher.label-to-train", enumOptions, false, "Label to train");
    setting->changed.connect([this](int value) {
        d_labelToTrain = value;
      });
    setting->setValue(0);

    Config::addSetting(setting);

    Config::getSetting<bool>("histogram-label-teacher.fixed-range")->track([this](bool val) {
        d_fixedRange = val;
      });

    Config::addAction("histogram-label-teacher.snap-train-image", "Snap Image", [this]()
                      {
                        if (d_yuvTrainImage.rows == 0)
                          d_snapshotRequested = true;
                        else
                          d_yuvTrainImage = cv::Mat{0,0,CV_8UC3};
                      });

    Config::addAction("histogram-label-teacher.set-seed-point", "Set Seed Point", [this](rapidjson::Value* val) {
        auto xMember = val->FindMember("x");
        auto yMember = val->FindMember("y");
        if (xMember == val->MemberEnd() || !xMember->value.IsInt() ||
            yMember == val->MemberEnd() || !yMember->value.IsInt())
        {
          log::error() << "Unable to parse x and y integral values from action JSON";
        }
        int x = xMember->value.GetInt(),
            y = yMember->value.GetInt();
        log::info("HistogramLabelTeacherBase") << "Setting seed point: " << x << " " << y;
        d_seedPoint = Eigen::Vector2i{x, y};

        d_mask = floodFill();
      });

    Config::addAction("histogram-label-teacher.train", "Train", [this]() {
        train(d_labelToTrain, d_mask);
      });

    Config::addAction("histogram-label-teacher.label", "Label", [this]() {
        d_labelRequested = !d_labelRequested;
      });

  }

  inline std::vector<std::shared_ptr<PixelLabel>> LabelTeacher::getLabels() const
  {
    return d_labels;
  }

  inline void LabelTeacher::setYUVTrainImage(cv::Mat yuvTrainImage)
  {
    d_yuvTrainImage = yuvTrainImage;
    d_snapshotRequested = false;
  }

  inline void LabelTeacher::setSeedPoint(Eigen::Vector2i point)
  {
    d_seedPoint = point;
  }

  inline cv::Mat LabelTeacher::floodFill() const
  {
    cv::Mat mask = cv::Mat::zeros(d_yuvTrainImage.rows + 2, d_yuvTrainImage.cols + 2, CV_8UC1);

    auto mfd = cv::Scalar(d_maxFloodDiff, d_maxFloodDiff, d_maxFloodDiff);
    cv::floodFill(d_yuvTrainImage, mask,
                  cv::Point{d_seedPoint.x(), d_seedPoint.y()}, cv::Scalar(255), 0,
                  mfd, mfd, 4 | (d_fixedRange ? cv::FLOODFILL_FIXED_RANGE : 0) | cv::FLOODFILL_MASK_ONLY | (255 << 8));

    return cv::Mat{mask, cv::Rect(1, 1, d_yuvTrainImage.cols, d_yuvTrainImage.rows)};
    
    return mask;
  }
  
  inline void LabelTeacher::train(unsigned labelIdx, cv::Mat const& mask)
  {
    ASSERT(mask.type() == CV_8UC1);
    ASSERT(d_yuvTrainImage.cols == mask.cols && d_yuvTrainImage.rows == mask.rows);

    for (int i = 0; i < mask.rows; ++i)
    {
      uint8_t const* trainImageRow = d_yuvTrainImage.ptr<uint8_t>(i);
      uint8_t const* maskRow = mask.ptr<uint8_t>(i);

      for (int j = 0; j < mask.cols; ++j)
        if (maskRow[j] != 0)
        {
          auto yuv = Colour::YCbCr{trainImageRow[j * 3 + 0], trainImageRow[j * 3 + 1], trainImageRow[j * 3 + 2]};
          auto bgr = yuv.toBgrInt();
          auto hsv = bgr2hsv(bgr);

          d_labels[labelIdx]->addSample(hsv);
        }
    }

    // TODO: breaks when we use Histo labels
    for (auto label : d_labels)
    {
      auto setting = Config::getSetting<Colour::hsvRange>(std::string("vision.pixel-labels.") + label->getName());
      auto rangeLabel = std::static_pointer_cast<RangePixelLabel>(label);
      setting->setValue(rangeLabel->getHSVRange());
    }

  }

  inline cv::Mat LabelTeacher::label(unsigned labelIdx) const
  {
    auto labelImg = cv::Mat{d_yuvTrainImage.rows, d_yuvTrainImage.cols, CV_8UC1};
    for (int i = 0; i < labelImg.rows; ++i)
    {
      uint8_t const* trainImageRow = d_yuvTrainImage.ptr<uint8_t>(i);
      uint8_t* labelRow = labelImg.ptr<uint8_t>(i);

      for (int j = 0; j < labelImg.cols; ++j)
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
