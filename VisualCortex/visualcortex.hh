#ifndef BOLD_VISUALCORTEX_HH
#define BOLD_VISUALCORTEX_HH

#include <Eigen/Core>

#include <map>
#include <opencv2/core/core.hpp>

#include "../LineFinder/linefinder.hh"
#include "../PixelLabel/pixellabel.hh"

class minIni;

namespace bold
{
  class DataStreamer;
  class ImageLabeller;

  template <typename TPixel>
  class ImagePasser;

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

    static bold::PixelLabel pixelLabelFromConfig(
      minIni const& ini,
      std::string objectName,
      int hue,        int hueRange,
      int saturation, int saturationRange,
      int value,      int valueRange
      );

    /** Process the provided image, extracting features. */
    void integrateImage(cv::Mat& cameraImage);

    /** Composes and enqueues a debugging image. */
    void streamDebugImage(cv::Mat cameraImage, DataStreamer* d_streamer);

    bool isBallVisible() const { return d_isBallVisible; }
    std::vector<Observation> observations() const { return d_observations; }
    std::vector<Observation> goalObservations() const { return d_goalObservations; }
    Observation ballObservation() const { return d_ballObservation; }

    std::vector<LineFinder::LineHypothesis> lines() const { return d_lines; }

    /** Gets the singleton instance of the VisualCortex. */
    static VisualCortex& getInstance();

  private:
    std::vector<Observation> d_observations;
    std::vector<Observation> d_goalObservations;
    Observation d_ballObservation;
    bool d_isBallVisible;

    PixelLabel d_goalLabel;
    PixelLabel d_ballLabel;
    PixelLabel d_fieldLabel;
    PixelLabel d_lineLabel;

    ImageLabeller* d_imageLabeller;
    cv::Mat d_labelledImage;

    LineFinder* d_lineFinder;

    ImagePasser<uchar>* d_imagePasser;
    LineDotPass<uchar>* d_lineDotPass;
    BlobDetectPass* d_blobDetectPass;
    CartoonPass* d_cartoonPass;
    LabelCountPass* d_labelCountPass;

    std::map<uchar,bold::PixelLabel> d_pixelLabelById;
    std::vector<LineFinder::LineHypothesis> d_lines;

    int d_minBallArea;
  };

  inline VisualCortex& VisualCortex::getInstance()
  {
    static VisualCortex instance;
    return instance;
  }

}

#endif
