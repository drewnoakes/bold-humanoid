#include "maskwalklinefinder.ih"

MaskWalkLineFinder::MaskWalkLineFinder()
: d_imageWidth(Config::getStaticValue<int>("camera.image-width")),
  d_imageHeight(Config::getStaticValue<int>("camera.image-height")),
  d_mask(d_imageHeight, d_imageWidth, CV_8UC1),
  d_trigTable()
{
  d_drThreshold         = Config::getSetting<double>("vision.line-detection.mask-walk.delta-r");
  d_dtThresholdDegs     = Config::getSetting<double>("vision.line-detection.mask-walk.delta-theta-degs");
  d_voteThreshold       = Config::getSetting<int>("vision.line-detection.mask-walk.min-votes");
  d_minLineLength       = Config::getSetting<int>("vision.line-detection.mask-walk.min-line-length");
  d_maxLineGap          = Config::getSetting<int>("vision.line-detection.mask-walk.max-line-gap");
  d_maxLineSegmentCount = Config::getSetting<int>("vision.line-detection.mask-walk.max-lines-returned");

  d_drThreshold->changed.connect([this](double value) { rebuild(); });
  d_dtThresholdDegs->changed.connect([this](double value) { rebuild(); });

  rebuild();
}
