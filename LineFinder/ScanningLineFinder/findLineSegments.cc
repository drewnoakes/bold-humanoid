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

  auto maxHeadDist = d_maxHeadDist->getValue();
  auto  worstFitAllowed = d_maxRMSFactor->getValue();

  // Matrix is col-major, so outersize is width
  for (unsigned k = 0; k < static_cast<unsigned>(linePointsMatrix.outerSize()); ++k)
    // Iterate over all elements in column k
    for (SparseMatrix<bool>::InnerIterator it(linePointsMatrix, k); it; ++it)
    {
      Vector2f point{it.col(), it.row()};
      auto bestFitingRegression = regressions.end();
      float bestFit = worstFitAllowed + 1;

      for (auto iter = begin(regressions); iter != end(regressions); ++iter)
      {
        auto& regression = *iter;
        float dist = (regression.head() - point.cast<float>()).norm();
        if (dist <= maxHeadDist)
        {
          //if (regression.getNPoints() > 1)
          {
            // Error should not be more than current RMSE
            auto fit = regression.fit(point);
            // only accept within 2 sigma
            if (fit > worstFitAllowed || fit > bestFit)
              continue;

            bestFit = fit;
            bestFitingRegression = iter;
          }
        }
      }

      if (bestFitingRegression != regressions.end())
      {
        bestFitingRegression->addPoint(point);
        bestFitingRegression->solve();
      }
      else
      {
        // Start new regression
        IncrementalRegression newRegression;
        newRegression.setSqError(100.0);

        newRegression.addPoint(point);
        regressions.push_back(newRegression);
      }
    }

  float minLength = d_minLength->getValue();
  float minCoverage = d_minCoverage->getValue();

  vector<IncrementalRegression,Eigen::aligned_allocator<IncrementalRegression>> acceptedRegressions;

  for (auto& regression : regressions)
  {
    if (regression.getNPoints() < 2 || regression.isVertical())
      continue;

    auto lineSegment = regression.getLineSegment();

    float length = lineSegment.length();
    if (length < minLength)
      continue;

    float coverage = float(regression.getNPoints()) / length;
    if (coverage < minCoverage)
      continue;

    acceptedRegressions.push_back(regression);
  }

  bool tryMerge = false;
  while (tryMerge)
  {
    tryMerge = false;
    // Try to connect
    vector<IncrementalRegression,Eigen::aligned_allocator<IncrementalRegression>> newAcceptedRegressions;
    for (unsigned i = 0; i < acceptedRegressions.size(); ++i)
    {
      auto reg1 = acceptedRegressions[i];
      for (unsigned j = i + 1; j < acceptedRegressions.size(); ++j)
      {
        auto reg2 = acceptedRegressions[j];
        auto beta1 = reg1.getBeta();
        auto beta2 = reg2.getBeta();
        if (std::abs(beta1(0) - beta2(0)) < 0.05 && std::abs(beta1(1) - beta2(1)) < 20)
        {
          reg1.merge(reg2);
          tryMerge = true;
          for (unsigned j2 = j + 1; j2 < acceptedRegressions.size(); ++j2)
            newAcceptedRegressions.push_back(acceptedRegressions[j2]);
          j = i = acceptedRegressions.size();
        }
      }
      newAcceptedRegressions.push_back(reg1);
    }
    acceptedRegressions = newAcceptedRegressions;
  }

  vector<LineSegment2i> lineSegments;
  for (auto& regression : acceptedRegressions)
    lineSegments.emplace_back(regression.getLineSegment().cast<int>());
  return lineSegments;
}
