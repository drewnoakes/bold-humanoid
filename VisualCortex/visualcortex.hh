#pragma once

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
  class CameraModel;
  class Debugger;
  class DataStreamer;
  class FieldMap;
  class HeadModule;
  class ImageLabeller;
  class LineFinder;
  class SequentialTimer;
  class Spatialiser;

  template <typename TPixel>
  class ImagePassRunner;

  template <typename TPixel>
  class ImagePassHandler;

  template <typename TPixel>
  class LineDotPass;
  class BlobDetectPass;
  class CartoonPass;
  class LabelCountPass;

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
                 std::shared_ptr<HeadModule> headModule,
                 minIni const& ini);

    std::map<std::string,std::vector<Control>> getControlsByFamily() const { return d_controlsByFamily; }

    /** Process the provided image, extracting features. */
    void integrateImage(cv::Mat& cameraImage, SequentialTimer& timer);

    /** Composes and enqueues a debugging image. */
    void streamDebugImage(cv::Mat cameraImage, std::shared_ptr<DataStreamer> streamer, SequentialTimer& timer);

    void setShouldDetectLines(bool val) { d_shouldDetectLines = val; }
    bool getShouldDetectLines() const { return d_shouldDetectLines; }

    void setShouldIgnoreAboveHorizon(bool val) { d_shouldIgnoreAboveHorizon = val; }
    bool getShouldIgnoreAboveHorizon() const { return d_shouldIgnoreAboveHorizon; }

    void setMinBallArea(unsigned val) { d_minBallArea = val; }
    unsigned getMinBallArea() const { return d_minBallArea; }

    void setStreamFramePeriod(unsigned val) { d_streamFramePeriod = val; }
    unsigned getStreamFramePeriod() const { return d_streamFramePeriod; }

    
    void setShouldDrawBlobs(bool val) { d_shouldDrawBlobs = val; }
    bool getShouldDrawBlobs() const { return d_shouldDrawBlobs; }

    void setShouldDrawLineDots(bool val) { d_shouldDrawLineDots = val; }
    bool getShouldDrawLineDots() const { return d_shouldDrawLineDots; }

    void setShouldDrawExpectedLines(bool val) { d_shouldDrawExpectedLines = val; }
    bool getShouldDrawExpectedLines() const { return d_shouldDrawExpectedLines; }

    void setShouldDrawObservedLines(bool val) { d_shouldDrawObservedLines = val; }
    bool getShouldDrawObservedLines() const { return d_shouldDrawObservedLines; }

    void setShouldDrawHorizon(bool val) { d_shouldDrawHorizon = val; }
    bool getShouldDrawHorizon() const { return d_shouldDrawHorizon; }
    
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

    bool d_shouldDetectLines;
    bool d_shouldIgnoreAboveHorizon;

    unsigned d_minBallArea;

    ImageType d_imageType;
    unsigned d_streamFramePeriod;
    bool d_shouldDrawBlobs;
    bool d_shouldDrawLineDots;
    bool d_shouldDrawExpectedLines;
    bool d_shouldDrawObservedLines;
    bool d_shouldDrawHorizon;
  };
}
