#include "visualcortex.ih"

bool VisualCortex::shouldMergeBallBlobs(Bounds2i const& larger, Bounds2i const& smaller)
{
  // Merge if the union would be more square, and the new maxDimension is not far off the original one

  Bounds2i combined = Bounds2i::merge(larger, smaller);


  cout << "largerBounds="<<largerBounds<<" smallerBounds="<<smallerBounds<<" unionBounds="<<unionBounds<<endl;

  double largerAspect = (double)larger.minDimension() / larger.maxDimension();
  double unionAspect = (double)combined.minDimension() / combined.maxDimension();

  cout << "unionAspect="<<unionAspect<<" largerAspect="<<largerAspect<<endl;

  if (unionAspect < largerAspect)
    return false;

  int maxDimension = max(larger.maxDimension(), smaller.maxDimension());

  int unionMaxDimension = combined.maxDimension();

  assert(unionMaxDimension >= maxDimension);

  cout << "unionMaxDimension="<<unionMaxDimension<<" maxDimension="<<maxDimension<<endl;

  return unionMaxDimension < maxDimension * 1.3;
}
