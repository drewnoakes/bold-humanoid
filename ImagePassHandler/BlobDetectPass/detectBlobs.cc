#include "blobdetectpass.ih"

#include "../../SequentialTimer/sequentialtimer.hh"

map<shared_ptr<PixelLabel>,vector<Blob>> const& BlobDetectPass::detectBlobs(SequentialTimer& timer)
{
  assert(d_rowIndices.size() > 1);

  // For each label that we're configured to look at
  for (shared_ptr<PixelLabel> const& pixelLabel : d_pixelLabels)
  {
    uchar pixelLabelId = pixelLabel->id();

    // Go through all runs and add them to the disjoint set

    // RunSets; one set of runSets for each label, each blob is a set of runs

    RunLengthCode& runsPerRow = d_runsPerRowPerLabel[pixelLabelId];

    DisjointSet<Run> rSet;

    // Just insert all runs of top row
    for (Run const& run : runsPerRow[d_rowIndices[0]])
    {
      rSet.insert(run);
    }

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

    set<set<Run>> runSets = rSet.getSubSets();

    auto& blobSet = d_blobsDetectedPerLabel[pixelLabel];
    blobSet.clear();

    // Convert sets of run-sets to sets of blob
    transform(runSets.begin(), runSets.end(),
              inserter(blobSet, blobSet.end()),
              runSetToBlob);

    // TODO: why isn't blobSet a set again? That would auto-sort
    std::sort(blobSet.begin(), blobSet.end(), greater<Blob>());

    timer.timeEvent(string(pixelLabel->name()));
  }

  return d_blobsDetectedPerLabel;
}
