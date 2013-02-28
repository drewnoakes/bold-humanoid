#include "blobdetectpass.hh"

using namespace bold;

vector<set<Blob>> BlobDetectPass::detectBlobs(std::vector<std::function<bool(Run const& a, Run const& b)>> unionPreds)
{
  vector<DisjointSet<Run>> rlSets(d_labelCount);

  // Go through all runs and add them to the disjoint sets

  // RunSets; one set of runSets for each label, each blob is a set of runs
  vector<set<Blob> > blobs(d_labelCount);

  // Loop over labels
  for (unsigned label = 0; label < d_labelCount; ++label)
  {
    RunLengthCode& rlCode = d_runsPerRowPerLabel[label];

//    cout << "rl code: " << label << " - " << rlCode.size() << endl;

    DisjointSet<Run> rSet;

    // Just insert all runs of top row
    for (Run& run : rlCode[0])
    {
      rSet.insert(run);
    }

    for (unsigned y = 1; y < d_imageHeight; ++y)
    {
      for (Run& run : rlCode[y])
      {
        rSet.insert(run);

        for (Run& run2 : rlCode[y - 1])
          if (unionPreds[label](run, run2))
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
