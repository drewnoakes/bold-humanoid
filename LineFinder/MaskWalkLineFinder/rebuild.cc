#include "maskwalklinefinder.ih"

void MaskWalkLineFinder::rebuild()
{
  d_tSteps = cvRound(CV_PI / d_dtThreshold);
  d_rSteps = cvRound(((d_imageWidth + d_imageHeight) * 2 + 1) / d_drThreshold);

  d_trigTable = vector<float>(d_tSteps*2);
  float idr = 1 / d_drThreshold;
  for (int n = 0; n < d_tSteps; n++)
  {
    d_trigTable[n*2] = (float)(cos((double)n*d_dtThreshold) * idr);
    d_trigTable[n*2+1] = (float)(sin((double)n*d_dtThreshold) * idr);
  }

  d_accumulator = Mat::zeros(d_tSteps, d_rSteps, CV_32SC1);
}