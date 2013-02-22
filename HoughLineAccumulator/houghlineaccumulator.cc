#include "houghlineaccumulator.ih"

void HoughLineAccumulator::add(int x, int y)
{
  uint16_t* rowBase = reinterpret_cast<uint16_t*>(d_accumulator.data);

  for (unsigned int t = 0; t < d_accumulatorThetaLen; t++)
  {
    double radius = x*d_sinCache[t] + y*d_cosCache[t];

    int radiusInt = (int)round(radius);

    // Recenter, as we have both positive and negative radius values
    radiusInt += d_accumulatorRadiusLen / 2;

    double theta = t/((double)d_accumulatorThetaLen-1) * M_PI;

    // Check within bounds
    if (radiusInt >= 0 && radiusInt < d_accumulatorRadiusLen)
    {
      auto ptr = rowBase + radiusInt;
      (*ptr)++;
    }

    rowBase += d_accumulatorRadiusLen;
  }
}

HoughLineAccumulator::HoughLineAccumulator(unsigned int xLength, unsigned int yLength, unsigned int accumulatorWidth)
  : d_accumulatorThetaLen(accumulatorWidth),
    d_accumulatorRadiusLen((unsigned int)ceil(sqrt(xLength*xLength + yLength+yLength))),
    d_xLength(xLength),
    d_yLength(yLength),
    d_sinCache(new double[d_accumulatorThetaLen]),
    d_cosCache(new double[d_accumulatorThetaLen])
{
  // cache the values of sin and cos for faster processing
  const double d_thetaStepRadians = M_PI / d_accumulatorThetaLen;
  for (unsigned int t = 0; t < d_accumulatorThetaLen; t++)
  {
    double theta = t*d_thetaStepRadians;
    d_sinCache[t] = sin(theta);
    d_cosCache[t] = cos(theta);
  }

  // Accumulator matrix has x=radius, y=theta (this is 90 degrees rotated from most examples on the web)
  d_accumulator = cv::Mat(cv::Size(d_accumulatorRadiusLen, d_accumulatorThetaLen), CV_16UC1, cv::Scalar(0));

  clear();
}

HoughLineAccumulator::~HoughLineAccumulator()
{
  delete[] d_sinCache;
  delete[] d_cosCache;
}

void HoughLineAccumulator::clear()
{
  d_accumulator = cv::Scalar(0);
}

cv::Mat HoughLineAccumulator::getMat()
{
  return d_accumulator;
}

double HoughLineAccumulator::getTheta(unsigned int y)
{
  return y * (M_PI / d_accumulatorThetaLen);
}

double HoughLineAccumulator::getRadius(unsigned int x)
{
  return x - (d_accumulatorRadiusLen / 2);
}
