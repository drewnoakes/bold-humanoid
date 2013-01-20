#include "blobdetector.ih"

vector<set<Blob > > BlobDetector::detectBlobs(cv::Mat const& labeledImage, unsigned char nLabels)
{
  vector<RunLengthCode> rlCodes = runLengthEncode(labeledImage, nLabels);
  
  vector<DisjointSet<Run>> rlSets(nLabels);
  
  // Go through all runs and add them to the disjoint sets

  // Function to check whether two runs are connected
  auto unionPred =
    [] (Run const& a, Run const& b)
    {
      return
        max(a.end.x(), b.end.x()) - min(a.start.x(), b.start.x()) <= a.length + b.length;
    };

  // RunSets; one set of runSets for each label, each blob is a set of runs
  vector<set<Blob> > blobs(nLabels);

  // Loop over labels
  for (unsigned label = 0; label < nLabels; ++label)
  {
    RunLengthCode& rlCode = rlCodes[label];

    DisjointSet<Run> rSet;

    // Just insert all runs of top row
    for (Run& run : rlCode[0])
    {
      rSet.insert(run);
    }

    for (unsigned y = 1; y < labeledImage.rows; ++y)
    {
      for (Run& run : rlCode[y])
      {
        rSet.insert(run);
        
        for (Run& run2 : rlCode[y - 1])
          if (unionPred(run, run2))
            rSet.merge(run, run2);
      }
    }

    set<set<Run> > runSets = rSet.getSubSets();

    set<Blob> bs;

    auto bsBegin = bs.begin();

    // Convert sets of sets runs to sets of blob
    auto bsEnd = transform(runSets.begin(), runSets.end(),
                           inserter(bs, bs.end()),
                           runSetToBlob);
    blobs[label] = bs;
  }

  return blobs;
}
