#include "maskwalklinefinder.ih"

MaskWalkLineFinder::MaskWalkLineFinder(int imageWidth, int imageHeight)
: d_imageWidth(imageWidth),
  d_imageHeight(imageHeight),
  d_mask(imageHeight, imageWidth, CV_8UC1),
  d_drThreshold(3),
  d_dtThreshold(1 * M_PI/180),
  d_voteThreshold(10),
  d_minLineLength(30),
  d_maxLineGap(30),
  d_maxLineSegmentCount(5),
  d_trigTable(),
  d_controls()
{
  rebuild();

  // TODO would a double/float valued controls be better here?
  Control drThreshold   = Control::createInt("Delta R",           (int)d_drThreshold,          [this](int const& value){ d_drThreshold = value;             rebuild(); });
  Control dtThreshold   = Control::createInt("Delta T (degs)",    d_dtThreshold * 180.0/CV_PI, [this](int const& value){ d_dtThreshold = value*CV_PI/180.0; rebuild(); });

  Control voteThreshold = Control::createInt("Min Votes",          d_voteThreshold,       [this](int const& value){ d_voteThreshold = value; });
  Control minLength     = Control::createInt("Min Line Length",    d_minLineLength,       [this](int const& value){ d_minLineLength = value; });
  Control maxGap        = Control::createInt("Max Line Gap",       d_maxLineGap,          [this](int const& value){ d_maxLineGap = value; });
  Control maxLines      = Control::createInt("Max Lines Returned", d_maxLineSegmentCount, [this](int const& value){ d_maxLineSegmentCount = value; });

  drThreshold.setLimitValues(1, 20);
  dtThreshold.setLimitValues(1, 20);
  voteThreshold.setLimitValues(1, 50);
  minLength.setLimitValues(1, (int)sqrt(imageWidth*imageWidth + imageHeight*imageHeight));
  maxGap.setLimitValues(0, 50);
  maxLines.setLimitValues(1, 20);

  drThreshold.setIsAdvanced(true);
  dtThreshold.setIsAdvanced(true);
  voteThreshold.setIsAdvanced(true);
  minLength.setIsAdvanced(true);
  maxGap.setIsAdvanced(true);
  maxLines.setIsAdvanced(true);

  d_controls = { drThreshold, dtThreshold, voteThreshold, minLength, maxGap, maxLines };
}
