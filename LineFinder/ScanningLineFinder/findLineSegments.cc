#include "scanninglinefinder.ih"

vector<LineSegment2i> ScanningLineFinder::findLineSegments(vector<Vector2i>& linePoints)
{
  SparseMatrix<bool, ColMajor> linePointsMatrix(d_cameraModel->imageHeight(), d_cameraModel->imageWidth());
  vector<Triplet<bool>> triplets;
  for (auto const& p : linePoints)
    triplets.emplace_back(p.y(), p.x(), true);

  // Make sure all points are sorted by column first, then row
  linePointsMatrix.setFromTriplets(begin(triplets), end(triplets));

  vector<IncrementalRegression,Eigen::aligned_allocator<IncrementalRegression>> regressions;
;
  // Maximum distance between a point can be away from the head of
  // line segment to be considered as a part of it
  float maxDist = d_maxHeadDist->getValue();;
  // Maximum y-error to consider point member of line segment
  float maxError = d_maxLineDist->getValue();

//   float maxRMSFactor = d_maxRMSFactor->getValue();

  // Matrix is col-major, so outersize is width
  for (unsigned k = 0; k < static_cast<unsigned>(linePointsMatrix.outerSize()); ++k)
    // Iterate over all elements in column k
    for (SparseMatrix<bool>::InnerIterator it(linePointsMatrix, k); it; ++it)
    {
      Vector2f point{it.col(), it.row()};
      auto closest = regressions.end();
      float error = 0;

      // Find line segment that 1) has head closest to point 2) has least residual between line and point
      // Minimum distance to head found so far
      float minDist = maxDist;
      // Minimum offset of line found so far
      float minError = 2 * maxError;

      for (auto iter = begin(regressions); iter != end(regressions); ++iter)
      {
        auto& regression = *iter;
        auto head = regression.head();
        float dist = (regression.head() - point.cast<float>()).norm();
        if (dist <= minDist)
        {
          if (regression.getNPoints() > 1)
          {
            // Error should not be more than current RMSE
            auto e = regression.fit(point);
            if (e > 1.0f)
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

      if (closest != regressions.end())
      {
        closest->addPoint(point);
        closest->solve();
      }
      else
      {
        // Start new regression
        IncrementalRegression newRegression;
        float e = d_maxLineDist->getValue();
        newRegression.setSqError(e * e);

        newRegression.addPoint(point);
        regressions.push_back(newRegression);
      }
    }

  float minLength = d_minLength->getValue();
  float minCoverage = d_minCoverage->getValue();

  vector<LineSegment2i> lineSegments;
  for (auto& regression : regressions)
  {
    if (regression.getNPoints() < 2)
      continue;

    auto lineSegment = regression.getLineSegment();

    float length = lineSegment.length();
    if (length < minLength)
      continue;

    float coverage = float(regression.getNPoints()) / length;
    if (coverage < minCoverage)
      continue;

    /*
    auto errors = (regState.X * regState.beta - regState.y).head(regState.n);
    auto rms = sqrt(errors.dot(errors) / regState.n);
    if (rms > maxRMSError)
      continue;
    */
    lineSegments.emplace_back(lineSegment.cast<int>());
  }
  return lineSegments;
}
