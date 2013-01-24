#include "blobdetector.ih"

vector<BlobDetector::RunLengthCode> BlobDetector::runLengthEncode(Mat const& labeledImage, unsigned char nLabels)
{
  vector<RunLengthCode> rlCodes;
  // Create a run length code for each label
  for (unsigned char l = 0; l < nLabels; ++l)
  {
    rlCodes.push_back(RunLengthCode());
    // a run length code is a vector of vectors of runs
    for (unsigned y = 0; y < labeledImage.rows; ++y)
      rlCodes[l].push_back(vector<Run>());

    cout << "rlcs: " << l << " " << rlCodes[l].size() << endl;
  }

  Run curRun(0,0);
  unsigned char curLabel = 0;

  for (unsigned y = 0; y < labeledImage.rows; ++y)
  {
    unsigned char const* row = labeledImage.ptr<unsigned char>(y);
    // We go one pixel outside of the row, as if image is padded with a column of zeros
    for (unsigned x = 0; x <= labeledImage.cols; ++x)
    {
      unsigned char label = x < labeledImage.cols ? row[x] : 0;

      // Check if we have a run boundary
      if (label != curLabel)
      {
        // Check whether this is the end of the current run
        if (curLabel != 0)
        {
          // Finished run
          curRun.end = Vector2i(x, y);
          curRun.length = x - curRun.start.x();
	  if (curLabel <= nLabels)
	    rlCodes[curLabel - 1][y].push_back(curRun);
        }

        // Check whether this is the start of a new run
        if (label != 0)
        {
          // Start new run
          curRun = Run(x, y);
        }

        curLabel = label;
      }
    }
  }

  return rlCodes;
}
