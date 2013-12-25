#pragma once

#include <Eigen/Core>

#include <functional>
#include <map>
#include <memory>
#include <opencv2/core/core.hpp>

#include "../geometry/LineSegment2i.hh"
#include "../PixelLabel/pixellabel.hh"
#include "../Setting/setting.hh"
#include "../util/meta.hh"

namespace bold
{
  struct Blob;
  class Camera;
  class CameraModel;
  class DataStreamer;
  class FieldMap;
  class HeadModule;
  class ImageLabeller;
  class LineFinder;
  class SequentialTimer;
  class Spatialiser;

  template<typename> class ImagePassRunner;
  template<typename> class ImagePassHandler;
  template<typename> class LineDotPass;
  class BlobDetectPass;
  class CartoonPass;
  class CompleteFieldEdgePass;
  class FieldEdgePass;
  class LabelCountPass;
  class PeriodicFieldEdgePass;

  enum class ImageType
  {
    None = 0,
    YCbCr = 1,
    RGB = 2,
    Cartoon = 3
  };

  enum class ImageGranularity
  {
    All = 0,
    Half = 1,
    Third = 2,
    Gradient = 3,
    Projected = 4
  };

  enum class FieldEdgeType
  {
    Complete = 0,
    Periodic = 1
  };

  /** Bold-humanoid's vision processing subsystem. */
  class VisualCortex
  {
  public:
    static bool shouldMergeBallBlobs(Bounds2i const& larger, Bounds2i const& smaller);

    VisualCortex(std::shared_ptr<Camera> camera,
                 std::shared_ptr<CameraModel> cameraModel,
                 std::shared_ptr<FieldMap> fieldMap,
                 std::shared_ptr<Spatialiser> spatialiser,
                 std::shared_ptr<HeadModule> headModule);

    /** Process the provided image, extracting features. */
    void integrateImage(cv::Mat& cameraImage, SequentialTimer& timer);

    /** Saves the provided image to a file, along with information about the current agent's state in a JSON file. */
    void saveImage(cv::Mat const& image);

    /** Composes and enqueues a debugging image. */
    void streamDebugImage(cv::Mat cameraImage, std::shared_ptr<DataStreamer> streamer, SequentialTimer& timer);

    void setShouldDetectLines(bool val) { d_shouldDetectLines->setValue(val); }
    bool getShouldDetectLines() const { return d_shouldDetectLines->getValue(); }

    void setShouldCountLabels(bool val) { d_shouldCountLabels->setValue(val); }
    bool getShouldCountLabels() const { return d_shouldCountLabels->getValue(); }

    void setShouldIgnoreAboveHorizon(bool val) { d_shouldIgnoreAboveHorizon->setValue(val); }
    bool getShouldIgnoreAboveHorizon() const { return d_shouldIgnoreAboveHorizon->getValue(); }

    void setMinBallArea(unsigned val) { d_minBallArea->setValue(val); }
    unsigned getMinBallArea() const { return d_minBallArea->getValue(); }

    void setMinGoalDimensionPixels(unsigned val) { d_minGoalDimensionPixels->setValue(val); }
    unsigned getMinGoalDimensionPixels() const { return d_minGoalDimensionPixels->getValue(); }

    void setStreamFramePeriod(unsigned val) { d_streamFramePeriod->setValue(val); }
    unsigned getStreamFramePeriod() const { return d_streamFramePeriod->getValue(); }

    void setShouldDrawBlobs(bool val) { d_shouldDrawBlobs->setValue(val); }
    bool getShouldDrawBlobs() const { return d_shouldDrawBlobs->getValue(); }

    void setShouldDrawLineDots(bool val) { d_shouldDrawLineDots->setValue(val); }
    bool getShouldDrawLineDots() const { return d_shouldDrawLineDots->getValue(); }

    void setShouldDrawExpectedLines(bool val) { d_shouldDrawExpectedLines->setValue(val); }
    bool getShouldDrawExpectedLines() const { return d_shouldDrawExpectedLines->getValue(); }

    void setShouldDrawExpectedLineEdges(bool val) { d_shouldDrawExpectedLineEdges->setValue(val); }
    bool getShouldDrawExpectedLineEdges() const { return d_shouldDrawExpectedLineEdges->getValue(); }

    void setShouldDrawObservedObjects(bool val) { d_shouldDrawObservedObjects->setValue(val); }
    bool getShouldDrawObservedObjects() const { return d_shouldDrawObservedObjects->getValue(); }

    void setShouldDrawObservedLines(bool val) { d_shouldDrawObservedLines->setValue(val); }
    bool getShouldDrawObservedLines() const { return d_shouldDrawObservedLines->getValue(); }

    void setShouldDrawHorizon(bool val) { d_shouldDrawHorizon->setValue(val); }
    bool getShouldDrawHorizon() const { return d_shouldDrawHorizon->getValue(); }

    void setShouldDrawFieldEdge(bool val) { d_shouldDrawFieldEdge->setValue(val); }
    bool getShouldDrawFieldEdge() const { return d_shouldDrawFieldEdge->getValue(); }

    void setShouldDrawCalibration(bool val) { d_shouldDrawCalibration->setValue(val); }
    bool getShouldDrawCalibration() const { return d_shouldDrawCalibration->getValue(); }

  private:
    bool canBlobBeBall(Blob const& ballBlob, Eigen::Vector2d* pos);

    template<typename T>
    std::shared_ptr<T> getHandler() { return meta::get<std::shared_ptr<T>>(d_imagePassHandlers); }

    std::shared_ptr<FieldMap> d_fieldMap;
    std::shared_ptr<Camera> d_camera;
    std::shared_ptr<CameraModel> d_cameraModel;
    std::shared_ptr<Spatialiser> d_spatialiser;

    std::shared_ptr<PixelLabel> d_goalLabel;
    std::shared_ptr<PixelLabel> d_ballLabel;
    std::shared_ptr<PixelLabel> d_fieldLabel;
    std::shared_ptr<PixelLabel> d_lineLabel;

    std::shared_ptr<ImageLabeller> d_imageLabeller;

    std::function<Eigen::Vector2i(int)> d_granularityFunction;

    /** A cached Mat, to be re-used each image pass. */
    cv::Mat d_labelledImage;

    std::shared_ptr<LineFinder> d_lineFinder;

    std::shared_ptr<ImagePassRunner<uchar>> d_imagePassRunner;

    std::tuple<
      std::shared_ptr<LineDotPass<uchar>>,
      std::shared_ptr<BlobDetectPass>,
      std::shared_ptr<CartoonPass>,
      std::shared_ptr<LabelCountPass>,
      std::shared_ptr<CompleteFieldEdgePass>,
      std::shared_ptr<PeriodicFieldEdgePass>
      > d_imagePassHandlers;

    std::shared_ptr<FieldEdgePass> d_fieldEdgePass;

    Setting<bool>* d_shouldDetectLines;
    Setting<bool>* d_shouldCountLabels;
    Setting<bool>* d_shouldIgnoreAboveHorizon;

    Setting<int>* d_minBallArea;
    Setting<int>* d_minGoalDimensionPixels;

    bool d_recordNextFrame;
    Setting<bool>* d_isRecordingFrames;

    Setting<ImageType>* d_imageType;
    Setting<int>* d_streamFramePeriod;

    Setting<Colour::bgr>* d_lineDotColour;
    Setting<Colour::bgr>* d_observedLineColour;
    Setting<Colour::bgr>* d_expectedLineColour;
    Setting<Colour::bgr>* d_horizonColour;
    Setting<Colour::bgr>* d_fieldEdgeColour;
    Setting<Colour::bgr>* d_calibrationColour;

    Setting<bool>* d_shouldDrawBlobs;
    Setting<bool>* d_shouldDrawLineDots;
    Setting<bool>* d_shouldDrawObservedLines;
    Setting<bool>* d_shouldDrawExpectedLines;
    Setting<bool>* d_shouldDrawExpectedLineEdges;
    Setting<bool>* d_shouldDrawHorizon;
    Setting<bool>* d_shouldDrawFieldEdge;
    Setting<bool>* d_shouldDrawCalibration;
    Setting<bool>* d_shouldDrawObservedObjects;
  };
}
