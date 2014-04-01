#include "scanninglinefinder.ih"

ScanningLineFinder::ScanningLineFinder(shared_ptr<CameraModel> cameraModel)
  : d_cameraModel(move(cameraModel))
{
  d_minLength = Config::getSetting<double>("vision.line-detection.scanning.min-length");
  d_minCoverage = Config::getSetting<double>("vision.line-detection.scanning.min-coverage");
  d_maxRMSFactor = Config::getSetting<double>("vision.line-detection.scanning.max-rms-factor");
  d_maxHeadDist = Config::getSetting<double>("vision.line-detection.scanning.max-head-dist");
  d_maxLineDist = Config::getSetting<double>("vision.line-detection.scanning.max-line-dist");
}
