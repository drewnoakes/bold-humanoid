#ifndef BOLD_VISUALCORTEX_HH
#define BOLD_VISUALCORTEX_HH

#include <Eigen/Core>

#include <map>
#include <memory>
#include <opencv2/core/core.hpp>

#include "../Control/control.hh"
#include "../geometry/LineSegment2i.hh"
#include "../PixelLabel/pixellabel.hh"

class minIni;

namespace bold
{
  class FieldMap;
  class CameraModel;
  class Debugger;
  class DataStreamer;
  class ImageLabeller;

  template <typename TPixel>
  class ImagePassRunner;

  template <typename TPixel>
  class ImagePassHandler;

  template <typename TPixel>
  class LineDotPass;

  class LineFinder;
  class BlobDetectPass;
  class CartoonPass;
  class LabelCountPass;

  class Spatialiser;

  enum class ImageType
  {
    None = 0,
    YCbCr = 1,
    RGB = 2,
    Cartoon = 3
  };

  /** Bold-humanoid's vision processing subsystem. */
  class VisualCortex
  {
  public:
    VisualCortex(std::shared_ptr<CameraModel> cameraModel,
                 std::shared_ptr<FieldMap> fieldMap,
                 std::shared_ptr<Spatialiser> spatialiser,
                 std::shared_ptr<Debugger> debugger,
                 minIni const& ini);

    std::map<std::string,std::vector<Control>> getControlsByFamily() const { return d_controlsByFamily; }

    /** Process the provided image, extracting features. */
    void integrateImage(cv::Mat& cameraImage);

    /** Composes and enqueues a debugging image. */
    void streamDebugImage(cv::Mat cameraImage, std::shared_ptr<DataStreamer> streamer);

  private:
    std::map<std::string,std::vector<Control>> d_controlsByFamily;

    std::shared_ptr<FieldMap> d_fieldMap;
    std::shared_ptr<CameraModel> d_cameraModel;
    std::shared_ptr<Spatialiser> d_spatialiser;
    std::shared_ptr<Debugger> d_debugger;

    std::shared_ptr<PixelLabel> d_goalLabel;
    std::shared_ptr<PixelLabel> d_ballLabel;
    std::shared_ptr<PixelLabel> d_fieldLabel;
    std::shared_ptr<PixelLabel> d_lineLabel;

    std::shared_ptr<ImageLabeller> d_imageLabeller;

    /** A cached Mat, to be re-used each image pass. */
    cv::Mat d_labelledImage;

    std::shared_ptr<LineFinder> d_lineFinder;

    std::shared_ptr<ImagePassRunner<uchar>> d_imagePassRunner;

    std::shared_ptr<LineDotPass<uchar>> d_lineDotPass;
    std::shared_ptr<BlobDetectPass> d_blobDetectPass;
    std::shared_ptr<CartoonPass> d_cartoonPass;
    std::shared_ptr<LabelCountPass> d_labelCountPass;

    std::map<uchar,bold::PixelLabel> d_pixelLabelById;

    int d_minBallArea;
    double d_goalRunUnionRatio;

    ImageType d_imageType;
    unsigned d_streamFramePeriod;
    bool d_shouldDrawBlobs;
    bool d_shouldDrawLineDots;
    bool d_shouldDrawExpectedLines;
    bool d_shouldDrawObservedLines;
    bool d_shouldDrawHorizon;
  };
}

#endif
