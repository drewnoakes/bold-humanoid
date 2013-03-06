#ifndef BOLD_EYE_HH
#define BOLD_EYE_HH

#include <LinuxDARwIn.h>
#include <Eigen/Core>

#include <map>
#include <opencv2/core/core.hpp>

#include "../DataStreamer/datastreamer.hh"
#include "../vision/ImageLabeller/imagelabeller.hh"
#include "../vision/ImagePasser/imagepasser.hh"
#include "../vision/ImagePassHandler/BlobDetectPass/blobdetectpass.hh"
#include "../vision/ImagePassHandler/CartoonPass/cartoonpass.hh"
#include "../vision/ImagePassHandler/LabelCountPass/labelcountpass.hh"
#include "../vision/ImagePassHandler/LineDotPass/linedotpass.hh"
#include "../vision/LineFinder/linefinder.hh"
#include "../vision/LUTBuilder/lutbuilder.hh"
#include "../vision/PixelFilterChain/pixelfilterchain.hh"
#include "../vision/PixelLabel/pixellabel.hh"

namespace bold
{
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

  /** Bold-humanoid's vision processing subsystem.
   *
   * This class utilises components from the 'vision' submodule.
   */
  class VisualCortex
  {
  public:
    VisualCortex()
    : d_minBallArea(8*8),
      d_isBallVisible(false),
      d_lineFinder(nullptr),
      d_imagePasser(nullptr),
      d_blobDetectPass(nullptr),
      d_cartoonPass(nullptr),
      d_labelCountPass(nullptr),
      d_lineDotPass(nullptr),
      d_pixelLabelById(),
      d_observations(),
      d_goalObservations(),
      d_ballObservation( )
    {}

    void initialise(minIni const& ini);

    static bold::PixelLabel pixelLabelFromConfig(
      minIni const& ini,
      std::string objectName,
      int hue,        int hueRange,
      int saturation, int saturationRange,
      int value,      int valueRange
    )
    {
      bold::Colour::hsvRange hsvRange;
      hsvRange.h      = ini.geti("Vision", objectName + "Hue",             hue);
      hsvRange.hRange = ini.geti("Vision", objectName + "HueRange",        hueRange);
      hsvRange.s      = ini.geti("Vision", objectName + "Saturation",      saturation);
      hsvRange.sRange = ini.geti("Vision", objectName + "SaturationRange", saturationRange);
      hsvRange.v      = ini.geti("Vision", objectName + "Value",           value);
      hsvRange.vRange = ini.geti("Vision", objectName + "ValueRange",      valueRange);

      return bold::PixelLabel(hsvRange, objectName);
    }

    /** Process the provided image, extracting features. */
    void integrateImage(cv::Mat& cameraImage, DataStreamer* streamer = 0);

   /** Gets the singleton instance of the VisualCortex. */
    static VisualCortex& getInstance()
    {
      static VisualCortex instance;
      return instance;
    }

    bool isBallVisible() const { return d_isBallVisible; }
    std::vector<Observation> observations() const { return d_observations; }
    std::vector<Observation> goalObservations() const { return d_goalObservations; }
    Observation ballObservation() const { return d_ballObservation; }

    std::vector<LineFinder::LineHypothesis> lines() { return d_lines; }

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
    LineFinder* d_lineFinder;
    PixelFilterChain d_pfChain;

    ImagePasser<uchar>* d_imagePasser;
    LineDotPass<uchar>* d_lineDotPass;
    BlobDetectPass* d_blobDetectPass;
    CartoonPass* d_cartoonPass;
    LabelCountPass* d_labelCountPass;

    std::map<uchar,bold::PixelLabel> d_pixelLabelById;
    std::vector<LineFinder::LineHypothesis> d_lines;

    int d_minBallArea;
  };
}

#endif