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
    float d_sigmaRange;

    bool d_snapshotRequested;
    bool d_labelRequested;
    bool d_fixedRange;

  };

}
