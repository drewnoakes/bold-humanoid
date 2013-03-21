#include "maskwalklinefinder.ih"

vector<LineSegment2i> MaskWalkLineFinder::findLineSegments(vector<Vector2i>& lineDots)
{
  // IDEA instead of clearing the mask, just use a different value each time around -- clear every 255 runs

  // shuffle lineDots to simulate drawing at random
  random_shuffle(lineDots.begin(), lineDots.end());

  // Reset cache memory
  d_accumulator = Scalar(0);
  d_mask = Scalar(0);

  const float* ttab = &d_trigTable[0];
  uchar* mask0 = d_mask.data;

  // Place line dots on the mask
  for (auto const& dot : lineDots)
    d_mask.at<uchar>(dot.y(), dot.x()) = 255;

  vector<LineSegment2i> segments;

  //
  // Process line dots
  //
  int dotIndex = (int)lineDots.size() - 1;
  while (dotIndex >= 0)
  {
    Vector2i dot = lineDots[dotIndex--];

    // Check if it has been excluded already (i.e. belongs to some other line)
    if (!mask0[dot.y()*d_imageWidth + dot.x()])
      continue;

    int maxVotes = d_voteThreshold-1;
    int maxTheta = 0;
    int* adata = (int*)d_accumulator.data;

    // Update accumulator, finding the most probable line during the sweep.
    for (int n = 0; n < d_tSteps; n++, adata += d_rSteps)
    {
      // r = x*cos(theta) + y*sin(theta)
      int r = cvRound(dot.x() * ttab[n*2] + dot.y() * ttab[n*2+1]);
      // recenter
      r += (d_rSteps - 1) / 2;
      int votes = ++adata[r];
      if (maxVotes < votes)
      {
        maxVotes = votes;
        maxTheta = n;
      }
    }

    // If the most votes we saw during the update of the accumulator is still
    // below our threshold, jump to processing the next dot.
    if (maxVotes < d_voteThreshold)
      continue;

    // From the current point walk in each direction along the found line and
    // extract the line segment
    float tcos = ttab[maxTheta*2];
    float tsin = -ttab[maxTheta*2+1];
    bool isVertical = fabs(tsin) > fabs(tcos);

    // We using fixed-point integer arithmetic. This 'shift' is the amount to
    // bump integers up, providing space for fractions of whole numbers.
    // Outcomes are shifted back down again to give integer results.
    // Values 'x' and 'y', when shifted, become 'i' and 'j'.
    // We only shift one axis, depending upon the orientation of the line.
    const int shift = 16;

    int i0, j0, di, dj;
    if (isVertical)
    {
      i0 = dot.x();
      j0 = (dot.y() << shift) + (1 << (shift-1));
      di = tsin > 0 ? 1 : -1;
      // y = (r - x*cos(theta))/sin(theta)
      dj = cvRound(tcos*(1 << shift)/fabs(tsin));
    }
    else
    {
      i0 = (dot.x() << shift) + (1 << (shift-1));
      j0 = dot.y();
      dj = tcos > 0 ? 1 : -1;
      // x = (r - y*sin(theta))/cos(theta)
      di = cvRound(tsin*(1 << shift)/fabs(tcos));
    }

    // For both ends...
    Vector2i lineEnds[2];
    for (int endIndex = 0; endIndex <= 1; endIndex++)
    {
      int gap = 0;

      if (endIndex != 0)
      {
        // Flip, for second end, and walk the opposite direction
        di = -di;
        dj = -dj;
      }

      // Walk along the line using fixed-point arithmetics,
      // stop at the image border or in case of too big gap
      for (int i = i0, j = j0; ; i += di, j += dj)
      {
        int x, y;

        if (isVertical)
        {
          x = i;
          y = j >> shift;
        }
        else
        {
          x = i >> shift;
          y = j;
        }

        if (x < 0 || x >= d_imageWidth || y < 0 || y >= d_imageHeight)
        {
          // Stop if we reach the edge of the image
          break;
        }

        // If on line
        if (mask0[y*d_imageWidth + x])
        {
          // The mask shows this is a line dot.
          // Reset the gap.
          gap = 0;
          // Update line end.
          lineEnds[endIndex].y() = y;
          lineEnds[endIndex].x() = x;
        }
        else if (++gap > d_maxLineGap)
        {
          // It's too long since we last saw a dot
          break;
        }
      }
    }

    // NOTE The length check only applies to the x or y component.
    bool isLongEnough = (lineEnds[1] - lineEnds[0]).cwiseAbs().maxCoeff() >= d_minLineLength;

    // Now walk the line again, and clean up the mask/accum
    for (int endIndex = 0; endIndex <= 1; endIndex++)
    {
      if (endIndex != 0)
      {
        // flip, for second end, and walk the opposite direction
        di = -di;
        dj = -dj;
      }

      // walk along the line using fixed-point arithmetics,
      // stop at the image border or in case of too big gap
      for (int i = i0, j = j0;; i += di, i += dj)
      {
        int x, y;

        if (isVertical)
        {
          x = i;
          y = j >> shift;
        }
        else
        {
          x = i >> shift;
          y = j;
        }

        if (x < 0 || x >= d_imageWidth || y < 0 || y >= d_imageHeight)
        {
          // Stop if we reach the edge of the image
          break;
        }

        // for each non-zero point:
        uchar* mdata = mask0 + y*d_imageWidth + x;
        if (*mdata)
        {
          if (isLongEnough)
          {
            // Remove this line from the accumulator
            adata = (int*)d_accumulator.data;
            for (int n = 0; n < d_tSteps; n++, adata += d_rSteps)
            {
              int r = cvRound(x * ttab[n*2] + y * ttab[n*2+1]);
              r += (d_rSteps - 1) / 2;
              adata[r]--; // subtract the vote
            }
          }
          // clear the mask element
          *mdata = 0;
        }

        if (y == lineEnds[endIndex].y() && x == lineEnds[endIndex].x())
        {
          // Stop if we reach the end of the line
          break;
        }
      }
    }

    if (isLongEnough)
    {
      segments.push_back(LineSegment2i(lineEnds[0], lineEnds[1]));

      if ((int)segments.size() >= d_maxLineSegmentCount)
      {
        // We've found enough segments now
        return segments;
      }
    }
  }

  return segments;
}
