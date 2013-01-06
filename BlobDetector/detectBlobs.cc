#include "blobdetector.ih"

void BlobDetector::detectBlobs(cv::Mat const& labeledImage, unsigned nLabels)
{
  vector<RunLengthCode> rlCodes = runLengthEncode(labeledImage);
  
  vector<DisjointSet<RunLength>> rlSets;

  // Go through all runs and add them to the disjoint sets

  // Function to check whether two runs are connected
  auto unionPred =
    [] (Run* a, Run* b)
    {
      return
        a->start.y() == b->start.y() + 1 &&
        max(a->end().x(), b->end().x()) - min(a->start.x(), b->start.x()) <= a->length + b->length;
    };

  for (RunLengthCode& rlCode : rlCodes)
  {
    DisjointSet<Run> rSet;

    for (Run& run : rlCode[0])
      rSet.insert(new DisjointSet<Run>::Element(&run));

    for (unsigned y = 1; y < labeledImage.cols; ++y)
      for (Run& run : rlCode[y])
	rSet.insert(new DisjointSet<Run>::Element(&run), unionPred);
  }

}
