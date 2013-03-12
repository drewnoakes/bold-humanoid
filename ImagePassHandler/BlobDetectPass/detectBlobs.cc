#include "blobdetectpass.ih"

map<PixelLabel,set<Blob>> BlobDetectPass::detectBlobs()
{
  map<PixelLabel,set<Blob>> blobsByLabel;

  // For each label that we're configured to look at
  for (BlobType const& blobType : d_blobTypes)
  {
    uchar pixelLabelId = blobType.pixelLabel.id();

    // Go through all runs and add them to the disjoint set

    // RunSets; one set of runSets for each label, each blob is a set of runs

    RunLengthCode& runsPerRow = d_runsPerRowPerLabel[pixelLabelId];

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
          if (blobType.unionPredicate(run, run2))
            rSet.merge(run, run2);
      }
    }

    set<set<Run>> runSets = rSet.getSubSets();

    set<Blob> blobSet;

    // Convert sets of sets runs to sets of blob
    transform(runSets.begin(), runSets.end(),
              inserter(blobSet, blobSet.end()),
              runSetToBlob);

    blobsByLabel[blobType.pixelLabel] = blobSet;
  }

  return blobsByLabel;
}
