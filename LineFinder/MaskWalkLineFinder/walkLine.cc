#include "maskwalklinefinder.ih"

// TODO determine expected thickness of line at the endpoints, and use this width when invoking the callback.

void MaskWalkLineFinder::walkLine(Vector2i const& start, float theta, bool forward, std::function<bool(int/*x*/,int/*y*/)> const& callback, uchar width)
{
  ASSERT(width);

  // From the current point walk in each direction along the found line and
  // extract the line segment
  float tcos = d_trigTable[theta*2];
  float tsin = -d_trigTable[theta*2+1];
  bool isVertical = fabs(tsin) > fabs(tcos);

  // We use fixed-point integer arithmetic. This 'shift' is the amount to
  // bump integers up, providing space for fractions of whole numbers.
  // Outcomes are shifted back down again to give integer results.
  // Values 'x' and 'y', when shifted, become 'i' and 'j'.
  // We only shift one axis, depending upon the orientation of the line.
  const int shift = 16;

  int i0, j0, di, dj;
  if (isVertical)
  {
    i0 = start.x();
    j0 = (start.y() << shift) + (1 << (shift-1));
    di = tsin > 0 ? 1 : -1;
    // y = (r - x*cos(theta))/sin(theta)
    dj = cvRound(tcos*(1 << shift)/fabs(tsin));
  }
  else
  {
    i0 = (start.x() << shift) + (1 << (shift-1));
    j0 = start.y();
    dj = tcos > 0 ? 1 : -1;
    // x = (r - y*sin(theta))/cos(theta)
    di = cvRound(tsin*(1 << shift)/fabs(tcos));
  }

  if (!forward)
  {
    di = -di;
    dj = -dj;
  }

  // Walk along the line using fixed-point arithmetics,
  // stop at the image border or in case of too big gap
  for (int i = i0, j = j0; ; i += di, j += dj)
  {
    int x, y, dx, dy;

    if (isVertical)
    {
      x = i;
      y = j >> shift;
      dx = 0;
      dy = 1;
    }
    else
    {
      x = i >> shift;
      y = j;
      dx = 1;
      dy = 0;
    }

    if (x < 0 || x >= d_imageWidth || y < 0 || y >= d_imageHeight)
    {
      // Stop if we reach the edge of the image
      break;
    }

    if (callback(x, y))
      break;

    while (dx < width && dy < width)
    {
      callback(x+dx, y+dy);
      callback(x-dx, y-dy);
      if (dx) dx++;
      if (dy) dy++;
    }
  }
}

