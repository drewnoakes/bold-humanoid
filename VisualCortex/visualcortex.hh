#pragma once

#include <Eigen/Core>

#include <functional>
#include <map>
#include <memory>
#include <opencv2/core/core.hpp>

#include "../LabelTeacher/labelteacher.hh"
#include "../geometry/LineSegment/LineSegment2/LineSegment2i/linesegment2i.hh"
#include "../PixelLabel/RangePixelLabel/rangepixellabel.hh"
#include "../PixelLabel/HistogramPixelLabel/histogrampixellabel.hh"
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

  class ImagePassRunner;
  class ImagePassHandler;
  class LineDotPass;
  class BlobDetectPass;
  class CartoonPass;
  class CompleteFieldEdgePass;
  class FieldEdgePass;
  class LabelCountPass;
  class PeriodicFieldEdgePass;
  class FieldHistogramPass;

  enum class ImageType
  {
    None = 0,
    YCbCr = 1,
    RGB = 2,
    Cartoon = 3,
    Teacher = 4
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
    static bool shouldMergeBallBlobs(Bounds2<ushort> const& larger, Bounds2<ushort> const& smaller);

    VisualCortex(std::shared_ptr<Camera> camera,
                 std::shared_ptr<CameraModel> cameraModel,
                 std::shared_ptr<DataStreamer> dataStreamer,
                 std::shared_ptr<Spatialiser> spatialiser,
                 std::shared_ptr<HeadModule> headModule);

    /** Process the provided image, extracting features. */
    void integrateImage(cv::Mat& cameraImage, SequentialTimer& timer, ulong thinkCycleNumber);

    /** Saves the provided image to a file, along with information about the current agent's state in a JSON file. */
    void saveImage(cv::Mat const& image, std::map<uchar,Colour::bgr>* palette);

    /** Composes and enqueues a debugging image. */
    void streamDebugImage(cv::Mat& cameraImage, SequentialTimer& timer);

    void setShouldDetectLines(bool val) { d_shouldDetectLines->setValue(val); }
    bool getShouldDetectLines() const { return d_shouldDetectLines->getValue(); }

    void setShouldCountLabels(bool val) { d_shouldCountLabels->setValue(val); }
    bool getShouldCountLabels() const { return d_shouldCountLabels->getValue(); }

    void setShouldIgnoreAboveHorizon(bool val) { d_shouldIgnoreAboveHorizon->setValue(val); }
    bool getShouldIgnoreAboveHorizon() const { return d_shouldIgnoreAboveHorizon->getValue(); }

    void setMinBallArea(unsigned val) { d_minBallAreaPixels->setValue(val); }
    unsigned getMinBallArea() const { return d_minBallAreaPixels->getValue(); }

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

    void setShouldDrawOcclusionEdge(bool val) { d_shouldDrawOcclusionEdge->setValue(val); }
    bool getShouldDrawOcclusionEdge() const { return d_shouldDrawOcclusionEdge->getValue(); }

    void setShouldDrawCalibration(bool val) { d_shouldDrawCalibration->setValue(val); }
    bool getShouldDrawCalibration() const { return d_shouldDrawCalibration->getValue(); }

  private:

    Maybe<Eigen::Vector2d> detectBall(std::vector<Blob>& ballBlobs, SequentialTimer& t);
    std::vector<Eigen::Vector2d,Eigen::aligned_allocator<Eigen::Vector2d>> detectGoal(std::vector<Blob>& goalBlobs, SequentialTimer& t);
    std::vector<Eigen::Vector2d,Eigen::aligned_allocator<Eigen::Vector2d>> detectPlayers(std::vector<Blob>& playerBlobs, SequentialTimer& t);

    bool canBlobBeBall(Blob const& ballBlob, Eigen::Vector2d& imagePos, Eigen::Vector3d& agentFramePos);
    bool canBlobBeGoal(Blob const& goalBlob, Eigen::Vector2d& pos);
    bool canBlobBePlayer(Blob const& playerBlob, Eigen::Vector2d& imagePos, Eigen::Vector3d& agentFramePos);

    std::shared_ptr<Camera> d_camera;
    std::shared_ptr<CameraModel> d_cameraModel;
    std::shared_ptr<DataStreamer> d_dataStreamer;
    std::shared_ptr<Spatialiser> d_spatialiser;

    std::vector<std::shared_ptr<PixelLabel>> d_pixelLabels;

    std::vector<std::shared_ptr<RangePixelLabel>> d_rangePixelLabels;
    std::vector<std::shared_ptr<HistogramPixelLabel<6>>> d_histogramPixelLabels;
    
    std::shared_ptr<ImageLabeller> d_imageLabeller;

    std::unique_ptr<LabelTeacher<6>> d_labelTeacher;

    std::function<Eigen::Matrix<uchar,2,1>(int)> d_granularityFunction;

    std::shared_ptr<LineFinder> d_lineFinder;

    std::shared_ptr<ImagePassRunner> d_imagePassRunner;

    std::shared_ptr<FieldEdgePass> d_fieldEdgePass;
    std::shared_ptr<FieldHistogramPass> d_fieldHistogramPass;
    std::shared_ptr<CartoonPass> d_cartoonPass;
    std::shared_ptr<BlobDetectPass> d_blobDetectPass;
    std::shared_ptr<LineDotPass> d_lineDotPass;

    Setting<bool>* d_shouldDetectLines;
    Setting<bool>* d_shouldCountLabels;
    Setting<bool>* d_shouldDetectBlobs;
    Setting<bool>* d_shouldIgnoreAboveHorizon;
    Setting<int>* d_maxBallFieldEdgeDistPixels;

    Setting<int>* d_minBallAreaPixels;
    Setting<Range<double>>* d_acceptedBallMeasuredSizeRatio;

    Setting<int>* d_minGoalDimensionPixels;
    Setting<int>* d_maxGoalFieldEdgeDistPixels;
    Setting<Range<double>>* d_acceptedGoalMeasuredWidthRatio;

    Setting<int>* d_minPlayerAreaPixels;
    Setting<int>* d_minPlayerLengthPixels;
    Setting<double>* d_goalieMarkerHeight;

    Setting<double>* d_fieldHistogramThreshold;

    bool d_saveNextYUVFrame;
    Setting<bool>* d_isRecordingYUVFrames;
    bool d_saveNextDebugFrame;

    Setting<ImageType>* d_imageType;
    Setting<int>* d_streamFramePeriod;

    Setting<Colour::bgr>* d_lineDotColour;
    Setting<Colour::bgr>* d_observedLineColour;
    Setting<Colour::bgr>* d_expectedLineColour;
    Setting<Colour::bgr>* d_horizonColour;
    Setting<Colour::bgr>* d_fieldEdgeColour;
    Setting<Colour::bgr>* d_fieldHistogramColour;
    Setting<Colour::bgr>* d_fieldHistogramIgnoredColour;
    Setting<Colour::bgr>* d_occlusionEdgeColour;
    Setting<Colour::bgr>* d_calibrationColour;

    Setting<bool>* d_shouldDrawBlobs;
    Setting<bool>* d_shouldDrawLineDots;
    Setting<bool>* d_shouldDrawObservedLines;
    Setting<bool>* d_shouldDrawExpectedLines;
    Setting<bool>* d_shouldDrawExpectedLineEdges;
    Setting<bool>* d_shouldDrawHorizon;
    Setting<bool>* d_shouldDrawFieldEdge;
    Setting<bool>* d_shouldDrawFieldHistogram;
    Setting<bool>* d_shouldDrawOcclusionEdge;
    Setting<bool>* d_shouldDrawCalibration;
    Setting<bool>* d_shouldDrawObservedObjects;

    Setting<bool>* d_ballBlobMergingEnabled;

    Setting<bool>* d_playerDetectionEnabled;
  };
}
