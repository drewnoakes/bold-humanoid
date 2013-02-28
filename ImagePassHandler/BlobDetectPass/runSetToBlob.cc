#include "blobdetectpass.hh"

using namespace bold;

Blob BlobDetectPass::runSetToBlob(set<Run> const& runSet)
{
  Blob b;
  // Put in constructor?
  b.ul = Vector2i(1e6,1e6);
  b.br = Vector2i(-1,-1);
  b.area = 0;
  b.mean << 0, 0;
  b.covar << 0, 0, 0, 0;

  b.runs = runSet;

  for (Run const& run : runSet)
  {
    // OPT: This can be optimized, we know the runs are ordered from top to bottom
    b.ul = b.ul.array().min(run.start.array());
    b.br = b.br.array().max(run.end.array());

    int y = run.start.y();

    b.area += run.length;
    b.mean.x() += run.length * ((run.end.x() - 1 + run.start.x()) / 2);
    b.mean.y() += run.length * y;

    // covar(0,0) = avg(x^2) - avg(x)^2
    // covar(0,1) = avg(x*y) - avg(x)*avg(y)
    // covar(1,0) = avg(x*y) - avg(x)*avg(y)
    // covar(0,0) = avg(y^2) - avg(y)^2


    // Sum_1^k n   = k(k+1)/2
    // Sum_1^k n^2 = k(k+1)(2k+1)/6
    // OPT: Can be precomputed and put in a LUT
    auto intSum = [](int k) { return k <= 0 ? 0 : k * (k + 1) / 2; };
    auto squareSum = [](int k) { return k <= 0 ? 0 : k * (k + 1) * (2*k + 1) / 6; };

    /*
    b.covar(0,0) += squareSum(run.end.x() - 1) - squareSum(run.start.x() - 1);
    b.covar(0,1) += (intSum(run.end.x() - 1) - intSum(run.start.x() - 1)) * y;
    b.covar(1,1) += run.length * y * y;
    */


    for (int x = run.start.x(); x < run.end.x(); ++x)
    {
      b.covar += Vector2f(x, y) * Vector2f(x, y).transpose();
      //b.covar(0,0) += x * x;
      //b.covar(0,1) += x * y;
      //b.covar(1,1) += y * y;
    }

  }

  b.mean /= b.area;

  b.covar /= b.area;

  b.covar -= b.mean * b.mean.transpose();

  //b.covar(1,0) = b.covar(0,1);

  return b;
}
