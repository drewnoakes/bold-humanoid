#include "scanninglinefinder.ih"

vector<LineSegment2i> ScanningLineFinder::findLineSegments(vector<Vector2i>& linePoints)
{
  SparseMatrix<bool, ColMajor> linePointsMatrix(d_cameraModel->imageHeight(), d_cameraModel->imageWidth());
  vector<Triplet<bool>> triplets;
  for (auto const& p : linePoints)
    triplets.emplace_back(p.y(), p.x(), true);

  linePointsMatrix.setFromTriplets(begin(triplets), end(triplets));

  // Make sure all points are sorted by column first, then row

  // Linear regression: Data generated according to: y = X . b + e
  // where X is design matrix with rows [x_i 1], y has elements y_i
  // (design matrix) and e is some error.
  // least squares solution:
  // b = (X^T X)^-1 X^T * y
  //   = (1/n \sum_i xi xi^T)^-1 (1/2 \sum_i xi y

  unsigned nRegressions = 0;

  // Structure keeping track of regression for a single line segment
  struct RegressionState
  {
    unsigned idx;
    Matrix2f xxSum;
    Vector2f xySum;
    unsigned n;
    Vector2f beta;
    float sqError;
    float rms;
    unsigned xStart;
    unsigned xEnd;
    Vector2i head;

    RegressionState(unsigned _idx, Matrix2f _xxSum, Vector2f _xySum, Vector2f _beta, unsigned _xStart, unsigned _xEnd, Vector2i _head)
      : idx{_idx},
      xxSum{move(_xxSum)}, xySum{move(_xySum)},
      n{1},
      beta{move(_beta)},
      sqError{0}, rms{0},
      xStart{_xStart}, xEnd{_xEnd},
      head{move(_head)}
    {}
  };

  vector<RegressionState,Eigen::aligned_allocator<RegressionState>> regStates;
  // Maximum distance between a point can be away from the head of
  // line segment to be considered as a part of it
  float maxDist = d_maxHeadDist->getValue();;
  // Maximum y-error to consider point member of line segment
  float maxError = d_maxLineDist->getValue();

  float maxRMSFactor = d_maxRMSFactor->getValue();

  // Matrix is col-major, so outersize is width
  for (unsigned k = 0; k < static_cast<unsigned>(linePointsMatrix.outerSize()); ++k)
    // Iterate over all elements in column k
    for (SparseMatrix<bool>::InnerIterator it(linePointsMatrix, k); it; ++it)
    {
      Vector2i point{it.col(), it.row()};
      auto x = Vector2f{point.x(), 1};
      auto xx = x * x.transpose();

      auto closest = regStates.end();
      float error = 0;

      // Find line segment that 1) has head closest to point 2) has least residual between line and point
      // Minimum distance to head found so far
      float minDist = maxDist;
      // Minimum offset of line found so far
      float minError = 2 * maxError;

      for (auto iter = begin(regStates); iter != end(regStates); ++iter)
      {
        auto& state = *iter;
        float dist = (state.head.cast<float>() - point.cast<float>()).norm();
        if (dist <= minDist)
        {
          if (state.n > 1)
          {
            // Error should not be more than current RMSE
            auto e = std::abs(state.beta.dot(x) - point.y());
            if (e > /*maxRMSFactor * */ state.rms)
              continue;
            error = e;
          }
          else
            error = maxError;

          if (error < minError)
          {
            minError = error;
            closest = iter;
            minDist = dist;
          }
        }
      }

      if (closest != regStates.end())
      {
        // Fit found; add point
        closest->xxSum += xx;
        closest->xySum += x * point.y();
        closest->n++;
        //if (error > .5 * maxError)
        {
          closest->beta = (closest->xxSum / closest->n).inverse() * (closest->xySum / closest->n);
          ++nRegressions;
        }
        closest->sqError += minError * minError;
        auto oldrms = closest->rms;
        closest->rms = sqrt(closest->sqError / closest->n);

        closest->xEnd = point.x() + 1;
        closest->head = point;
      }
      else
      {
        // Start new line segment
        RegressionState newState{regStates.size(), xx, x * point.y(), Vector2f{0, point.y()}, point.x() + 1, point.x() + 1, point};;
        regStates.push_back(newState);
      }
    }

  float minLength = d_minLength->getValue();
  float minCoverage = d_minCoverage->getValue();

  vector<LineSegment2i> lineSegments;
  for (auto& regState : regStates)
  {
    regState.beta =
      (regState.xxSum / regState.n).inverse() *
      (regState.xySum / regState.n);

    Vector2i p1(regState.xStart, regState.beta.dot(Vector2f(regState.xStart, 1)));
    Vector2i p2(regState.xEnd, regState.beta.dot(Vector2f(regState.xEnd, 1)));

    float length = (p2 - p1).norm();
    if (length < minLength)
      continue;

    float coverage = float(regState.n) / length;
    if (coverage < minCoverage)
      continue;

    /*
    auto errors = (regState.X * regState.beta - regState.y).head(regState.n);
    auto rms = sqrt(errors.dot(errors) / regState.n);
    if (rms > maxRMSError)
      continue;
    */
    lineSegments.emplace_back(p1, p2);
  }
  return lineSegments;
}
