#include "visualcortex.ih"

bool VisualCortex::shouldMergeBallBlobs(Blob& larger, Blob& smaller)
{
  // Merge if the union would be more square, and the new maxDimension is not far off the original one

  Bounds2i largerBounds = larger.bounds();
  Bounds2i smallerBounds = smaller.bounds();

  Bounds2i unionBounds = Bounds2i::merge(largerBounds, smallerBounds);

  cout << "largerBounds="<<largerBounds<<" smallerBounds="<<smallerBounds<<" unionBounds="<<unionBounds<<endl;

  double largerAspect = (double)largerBounds.minDimension() / largerBounds.maxDimension();
  double unionAspect = (double)unionBounds.minDimension() / unionBounds.maxDimension();

  cout << "unionAspect="<<unionAspect<<" largerAspect="<<largerAspect<<endl;

  if (unionAspect < largerAspect)
    return false;

  int maxDimension = max(largerBounds.maxDimension(), smallerBounds.maxDimension());

  int unionMaxDimension = unionBounds.maxDimension();

  assert(unionMaxDimension >= maxDimension);

  cout << "unionMaxDimension="<<unionMaxDimension<<" maxDimension="<<maxDimension<<endl;

  return unionMaxDimension < maxDimension * 1.3;
}
