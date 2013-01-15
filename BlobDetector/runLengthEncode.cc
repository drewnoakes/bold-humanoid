#include "blobdetector.ih"

vector<BlobDetector::RunLengthCode> BlobDetector::runLengthEncode(Mat const& labeledImage, unsigned char nLabels)
{
  cout << "nLabels: " << nLabels << endl;
  vector<RunLengthCode> rlCodes(nLabels);
  for (unsigned char l = 0; l < nLabels; ++nLabels)
    rlCodes[l] = RunLengthCode(labeledImage.rows);

  Run curRun(0,0);
  unsigned char curLabel = 0;

  cout << "Runlength encoding.." << endl;

  for (unsigned y = 0; y < labeledImage.rows; ++y)
  {
    cout << y << "\r";

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

  cout << "Done!" << endl;
}
