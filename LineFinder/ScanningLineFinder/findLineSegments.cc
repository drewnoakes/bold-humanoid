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
  };

  vector<RegressionState,Eigen::aligned_allocator<RegressionState>> regStates;
  // Maximum distance between a point can be away from the head of
  // line segment to be considered as a part of it
  float maxDist = d_maxHeadDist->getValue();;
  // Maximum y-error to consider point member of line segment
  float maxError = d_maxLineDist->getValue();

  // Matrix is col-major, so outersize is width
  for (unsigned k = 0; k < linePointsMatrix.outerSize(); ++k)
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
        if (dist < minDist)
        {
          if (state.n > 1)
          {
            // Error should not be more than current RMSE
            float e = abs(state.beta.dot(x) - point.y());
            if (fabs(e) > state.rms)
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
        if (error > .5 * maxError)
        {
          auto oldBeta = closest->beta;
          closest->beta = (closest->xxSum / closest->n).inverse() * (closest->xySum / closest->n);
          ++nRegressions;
        }
        closest->sqError += error * error;
        closest->rms = sqrt(closest->sqError / closest->n);
        closest->xEnd = point.x() + 1;
        closest->head = point;
      }
      else
      {
        // Start new line segment
        RegressionState newState;
        newState.idx = regStates.size() + 1;
        //newState.X = MatrixXf(linePoints.size(), 2);
        //newState.y = VectorXf(linePoints.size());
        //newState.X.row(0) = x.transpose();
        //newState.y(0) = point.y();
        newState.xxSum = xx;
        newState.xySum = x * point.y();
        newState.n = 1;
        newState.beta = Vector2f(0,point.y());
        newState.sqError = 0;
        newState.rms = 0;
        newState.xEnd = (newState.xStart = point.x()) + 1;
        newState.head = point;
        regStates.push_back(newState);
      }
    }

  float minLength = d_minLength->getValue();
  float minCoverage = d_minCoverage->getValue();
//   float maxRMSError = d_maxRMSError->getValue();

  vector<LineSegment2i> lineSegments;
  for (auto& regState : regStates)
  {
    auto oldBeta = regState.beta;
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
    lineSegments.push_back(LineSegment2i(p1, p2));
  }
  return lineSegments;
}
