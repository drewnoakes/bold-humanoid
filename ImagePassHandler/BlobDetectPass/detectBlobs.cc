#include "blobdetectpass.hh"

using namespace bold;
using namespace std;

vector<set<Blob>> BlobDetectPass::detectBlobs()
{
  vector<set<Blob>> blobsByLabel(d_labelCount);

  // Loop over labels
  for (unsigned label = 0; label < d_labelCount; ++label)
  {
    // Go through all runs and add them to the disjoint set

    // RunSets; one set of runSets for each label, each blob is a set of runs

    RunLengthCode& runsPerRow = d_runsPerRowPerLabel[label];

    DisjointSet<Run> rSet;

    // Just insert all runs of top row
    for (Run& run : runsPerRow[0])
    {
      rSet.insert(run);
    }

    // From the second row on...
    for (unsigned y = 1; y < d_imageHeight; ++y)
    {
      for (Run& run : runsPerRow[y])
      {
        rSet.insert(run);

        // Attempt to merge this run with runs in the row above
        for (Run& run2 : runsPerRow[y - 1])
          if (d_unionPredicateByLabel[label](run, run2))
            rSet.merge(run, run2);
      }
    }

    set<set<Run>> runSets = rSet.getSubSets();

    set<Blob> bs;

    // Convert sets of sets runs to sets of blob
    transform(runSets.begin(), runSets.end(),
              inserter(bs, bs.end()),
              runSetToBlob);

    blobsByLabel[label] = bs;
  }

  return blobsByLabel;
}
