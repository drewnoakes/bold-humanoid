#include "maskwalklinefinder.ih"

vector<LineSegment2i> MaskWalkLineFinder::findLineSegments(vector<Vector2i>& lineDots)
{
  // IDEA instead of clearing the mask, just use a different value each time around -- clear every 255 runs

  // load settings values as locals
  int voteThreshold = d_voteThreshold->getValue();
  int minLineLength = d_minLineLength->getValue();
  int maxLineGap = d_maxLineGap->getValue();
  int maxLineSegmentCount = d_maxLineSegmentCount->getValue();

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

    int maxVotes = voteThreshold-1;
    int maxTheta = 0;
    int* adata = reinterpret_cast<int*>(d_accumulator.data);

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
    if (maxVotes < voteThreshold)
      continue;

    // For both ends...
    LineSegment2i::Point lineEnds[2];
    for (int endIndex = 0; endIndex <= 1; endIndex++)
    {
      int gap = 0;
      auto pred = [&](int x, int y) {
        // If on line
        if (mask0[y*d_imageWidth + x])
        {
          // The mask shows this is a line dot.
          // Reset the gap.
          gap = 0;
          // Update line end.
          lineEnds[endIndex].y() = y;
          lineEnds[endIndex].x() = x;
          return false;
        }
        else if (++gap > maxLineGap)
        {
          // It's too long since we last saw a dot
          return true;
        }
        return false;
      };
      bool forward = endIndex == 0;
      walkLine(dot, maxTheta, forward, pred);
    }

    // NOTE The length check only applies to the x or y component.
    bool isLongEnough = (lineEnds[1] - lineEnds[0]).cwiseAbs().maxCoeff() >= minLineLength;

    // Now walk the line again, and clean up the mask/accum
    for (int endIndex = 0; endIndex <= 1; endIndex++)
    {
      auto pred = [&](int x, int y) {
        uchar* mdata = mask0 + y*d_imageWidth + x;
        if (*mdata)
        {
          if (isLongEnough)
          {
            // Remove this line from the accumulator
            int* adata = reinterpret_cast<int*>(d_accumulator.data);
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
          return true;
        }
        return false;
      };
      bool forward = endIndex == 0;
      walkLine(dot, maxTheta, forward, pred, /*width=*/2);
    }

    if (isLongEnough)
    {
      segments.emplace_back(lineEnds[0], lineEnds[1]);

      if ((int)segments.size() >= maxLineSegmentCount)
      {
        // We've found enough segments now
        return segments;
      }
    }
  }

  return segments;
}
