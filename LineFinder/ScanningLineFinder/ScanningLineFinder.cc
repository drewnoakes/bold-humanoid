#include "scanninglinefinder.ih"

ScanningLineFinder::ScanningLineFinder()
{
  d_minLength = Config::getSetting<double>("vision.line-detection.scanning.min-length");
  d_minCoverage = Config::getSetting<double>("vision.line-detection.scanning.min-coverage");
  d_maxRMSError = Config::getSetting<double>("vision.line-detection.scanning.max-rms-error");
  d_maxHeadDist = Config::getSetting<double>("vision.line-detection.scanning.max-head-dist");
  d_maxLineDist = Config::getSetting<double>("vision.line-detection.scanning.max-line-dist");
}