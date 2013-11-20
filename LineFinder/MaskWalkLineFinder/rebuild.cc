#include "maskwalklinefinder.ih"

void MaskWalkLineFinder::rebuild()
{
  auto dtThresholdRads = Math::degToRad(d_dtThresholdDegs->getValue());
  auto drThreshold = d_drThreshold->getValue();

  d_tSteps = cvRound(CV_PI / dtThresholdRads);
  d_rSteps = cvRound(((d_imageWidth + d_imageHeight) * 2 + 1) / drThreshold);

  d_trigTable = vector<float>(d_tSteps*2);
  float idr = 1 / drThreshold;
  for (int n = 0; n < d_tSteps; n++)
  {
    d_trigTable[n*2] = (float)(cos((double)n*dtThresholdRads) * idr);
    d_trigTable[n*2+1] = (float)(sin((double)n*dtThresholdRads) * idr);
  }

  d_accumulator = Mat::zeros(d_tSteps, d_rSteps, CV_32SC1);
}
