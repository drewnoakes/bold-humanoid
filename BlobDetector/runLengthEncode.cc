#include "blobdetector.ih"

map<unsigned, RunLengthCode> BlobDetector::runLengthEncode(Mat const& labeledImage, unsigned nLabels)
{
  vector<RunLengthCode> rlCodes(nLabels);

  Run curRun;
  unsigned char curLabel = 0;

  for (unsigned y = 0; y < img.rows; ++y)
  {
    
    unsigned char const* row = img.ptr<unsigned char>(y);
    // We go one pixel outside of the row, as if image is padded with a column of zeros
    for (unsigned x = 0; x <= img.cols; ++x)
    {
      label = x < img.cols ? row[x] : 0;

      // Check if we have a run boundary
      if (label != curLabel)
      {
	// Check whether this is the end of the current run
	if (curLabel != 0)
	{
	  // Finished run
	  curRun.length = x - curRun.start.x();
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

}
