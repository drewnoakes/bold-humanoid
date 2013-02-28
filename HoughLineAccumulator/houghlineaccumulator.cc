#include "houghlineaccumulator.ih"

void HoughLineAccumulator::add(int x, int y)
{
  d_count++;
  uint16_t* rowBase = reinterpret_cast<uint16_t*>(d_accumulator.data);
  int halfAccRadLen = d_accumulatorRadiusLen / 2;

  for (unsigned int t = 0; t < d_accumulatorThetaLen; t++)
  {
    // TODO store sin/cos in adjacent pairs for perf
    double radius = x*d_sinCache[t] + y*d_cosCache[t];

    int radiusInt = (int)round(radius);

    // Recenter, as we have both positive and negative radius values
    radiusInt += halfAccRadLen;

    // Check within bounds
    if (radiusInt >= 0 && radiusInt < d_accumulatorRadiusLen)
    {
      auto ptr = rowBase + radiusInt;
      // Add a vote to this bin
      (*ptr)++;
    }

    // Move to the next row
    rowBase += d_accumulatorRadiusLen;
  }
}

HoughLineAccumulator::HoughLineAccumulator(unsigned int xLength, unsigned int yLength, unsigned int accumulatorHeight)
  : d_accumulatorThetaLen(accumulatorHeight),
    // TODO the accumulator doesn't need to be quite this wide -- the negative side can be narrower (positive side is correct)
    d_accumulatorRadiusLen(2 * (unsigned int)ceil(sqrt(xLength*xLength + yLength*yLength))),
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

double HoughLineAccumulator::getTheta(int y)
{
  return y * (M_PI / d_accumulatorThetaLen);
}

double HoughLineAccumulator::getRadius(int x)
{
  return x - ((int)d_accumulatorRadiusLen / 2);
}
