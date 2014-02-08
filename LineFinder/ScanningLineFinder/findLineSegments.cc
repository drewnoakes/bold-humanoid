#include "scanninglinefinder.ih"

vector<LineSegment2i> ScanningLineFinder::findLineSegments(vector<Vector2i>& lineDots)
{
  // Make sure all dots are sorted by column first, then row
  sort(lineDots.begin(), lineDots.end(), [](Vector2i const& a, Vector2i const& b) {
      return a.x() < b.x() || (a.x() == b.x() && a.y() < b.y());
    });

  struct RegressionState
  {
    Matrix2d xxSum;
    Vector2d xySum;
    unsigned n;
    Vector2d beta;
    unsigned xStart;
    unsigned xEnd;
  };

  vector<RegressionState,Eigen::aligned_allocator<RegressionState>> regStates;

  double maxError = 5;

  auto x = Vector2d(0, 1);
  auto xx = x * x.transpose();
  for (auto const& dot : lineDots)
  {
    x.x() = dot.x();
    auto bestFit = regStates.end();
    double bestError = maxError;
    // Go through current linesegments and check if one fits
    for (auto iter = regStates.begin(); iter < regStates.end(); ++iter)
    {
      auto& state = *iter;
      double error = fabs(state.beta.dot(x) - dot.y());
      if (error < maxError && error < bestError)
      {
        bestError = error;
        bestFit = iter;
      }
    }

    if (bestFit != regStates.end())
    {
      //cout << "Best fit for: " << dot.transpose() << ": " << bestFit->beta.transpose() << endl;
      // Fit found; add dot
      bestFit->xxSum += xx;
      bestFit->xySum += x * dot.y();
      bestFit->n++;
      bestFit->beta = (bestFit->xxSum / bestFit->n).inverse() * (bestFit->xySum / bestFit->n);
      bestFit->xEnd = dot.x() + 1;
    }
    else
    {
      // Start new line segment
      RegressionState newState;
      newState.xxSum = xx;
      newState.xySum = x * dot.y();
      newState.n = 1;
      newState.beta = Vector2d(0,dot.y());
      newState.xEnd = (newState.xStart = dot.x()) + 1;
      regStates.push_back(newState);
      //cout << "Starting new line: " << newState.beta.transpose() << endl;
    }
  }

  vector<LineSegment2i> lineSegments;
  for (auto const& regState : regStates)
  {
    Vector2i p1(regState.xStart, regState.beta.dot(Vector2d(regState.xStart, 1)));
    Vector2i p2(regState.xEnd, regState.beta.dot(Vector2d(regState.xEnd, 1)));
    double length = (p2 - p1).norm();
      double coverage = double(regState.n) / length;
    if (coverage < 0.1 || length < 5)
      continue;
    lineSegments.push_back(LineSegment2i(p1, p2));
  }
  return lineSegments;
}
