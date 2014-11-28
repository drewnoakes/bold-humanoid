#include "blobdetectpass.hh"

using namespace bold;
using namespace std;

map<shared_ptr<PixelLabel>,vector<Blob>> const& BlobDetectPass::detectBlobs(SequentialTimer& timer)
{
  if (d_rowIndices.size() < 2)
    return d_blobsDetectedPerLabel;

  // For each label that we're configured to look at
  for (auto pixelLabel : d_pixelLabels)
  {
    timer.enter(pixelLabel->getName());
    auto pixelLabelId = (uint8_t)pixelLabel->getID();

    // Go through all runs and add them to the disjoint set

    // RunSets; one set of runSets for each label, each blob is a set of runs

    RunLengthCode& runsPerRow = d_runsPerRowPerLabel[pixelLabelId];

    DisjointSet<Run> rSet;

    // Just insert all runs of first (bottom) row
    for (Run const& run : runsPerRow[d_rowIndices[0]])
      rSet.insert(run);

    // From the second row on...
    for (unsigned i = 1; i < d_rowIndices.size(); ++i)
    {
      unsigned y = d_rowIndices[i];

      for (Run& run : runsPerRow[y])
      {
        rSet.insert(run);

        // Attempt to merge this run with runs in the row above
        for (Run& run2 : runsPerRow[d_rowIndices[i - 1]])
          if (run.overlaps(run2))
            rSet.merge(run, run2);
      }
    }

    timer.timeEvent("Build Run Set");

    set<set<Run>> runSets = rSet.getSubSets();

    timer.timeEvent("Get SubSets");

    auto& blobSet = d_blobsDetectedPerLabel[pixelLabel];
    blobSet.clear();

    // Convert sets of run-sets to sets of blob
    transform(runSets.begin(), runSets.end(),
              inserter(blobSet, blobSet.end()),
              runSetToBlob);

    timer.timeEvent("Convert");

    timer.exit();
  }

  return d_blobsDetectedPerLabel;
}
