#ifndef BOLD_VISUALCORTEX_HH
#define BOLD_VISUALCORTEX_HH

#include <Eigen/Core>

#include <map>
#include <memory>
#include <opencv2/core/core.hpp>

#include "../Control/control.hh"
#include "../LineFinder/linefinder.hh"
#include "../PixelLabel/pixellabel.hh"
#include "../geometry/LineSegment2i.hh"

class minIni;

namespace bold
{
  class DataStreamer;
  class ImageLabeller;

  template <typename TPixel>
  class ImagePassRunner;

  template <typename TPixel>
  class ImagePassHandler;

  template <typename TPixel>
  class LineDotPass;

  class BlobDetectPass;
  class CartoonPass;
  class LabelCountPass;

  enum ObsType
  {
    O_BALL,
    O_GOAL_POST,
    O_LEFT_GOAL_POST,
    O_RIGHT_GOAL_POST
  };

  struct Observation
  {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    ObsType type;
    Eigen::Vector2f pos;
  };

  /** Bold-humanoid's vision processing subsystem. */
  class VisualCortex
  {
  public:
    VisualCortex();

    void initialise(minIni const& ini);

    std::map<std::string,std::vector<Control>> getControlsByFamily() const { return d_controlsByFamily; }

    /** Process the provided image, extracting features. */
    void integrateImage(cv::Mat& cameraImage);

    /** Composes and enqueues a debugging image. */
    void streamDebugImage(cv::Mat cameraImage, DataStreamer* streamer);

    bool isBallVisible() const { return d_isBallVisible; }
    std::vector<Observation> observations() const { return d_observations; }
    std::vector<Observation> goalObservations() const { return d_goalObservations; }
    Observation ballObservation() const { return d_ballObservation; }

    std::vector<LineSegment2i> lines() const { return d_lines; }

    /** Gets the singleton instance of the VisualCortex. */
    static VisualCortex& getInstance();

  private:
    std::map<std::string,std::vector<Control>> d_controlsByFamily;

    std::vector<Observation> d_observations;
    std::vector<Observation> d_goalObservations;
    Observation d_ballObservation;
    bool d_isBallVisible;

    std::shared_ptr<PixelLabel> d_goalLabel;
    std::shared_ptr<PixelLabel> d_ballLabel;
    std::shared_ptr<PixelLabel> d_fieldLabel;
    std::shared_ptr<PixelLabel> d_lineLabel;

    ImageLabeller* d_imageLabeller;
    cv::Mat d_labelledImage;

    LineFinder* d_lineFinder;

    ImagePassRunner<uchar>* d_imagePassRunner;
    LineDotPass<uchar>* d_lineDotPass;
    BlobDetectPass* d_blobDetectPass;
    CartoonPass* d_cartoonPass;
    LabelCountPass* d_labelCountPass;

    std::map<uchar,bold::PixelLabel> d_pixelLabelById;
    std::vector<LineSegment2i> d_lines;

    int d_minBallArea;
  };

  inline VisualCortex& VisualCortex::getInstance()
  {
    static VisualCortex instance;
    return instance;
  }
}

#endif
