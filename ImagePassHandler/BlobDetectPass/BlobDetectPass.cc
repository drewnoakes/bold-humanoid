#include "blobdetectpass.hh"

using namespace bold;
using namespace std;

BlobDetectPass::BlobDetectPass(ushort imageWidth, ushort imageHeight, vector<shared_ptr<PixelLabel>> const& pixelLabels)
  : ImagePassHandler("BlobDetectPass"),
    d_imageHeight(imageHeight),
    d_imageWidth(imageWidth),
    d_pixelLabels(pixelLabels),
    d_runsPerRowPerLabel()
{
  // Create a run length code for each label
  for (auto const& pixelLabel : pixelLabels)
  {
    uint8_t pixelLabelId = (uint8_t)pixelLabel->getID();

    // A RunLengthCode is a vector of vectors of runs
    d_runsPerRowPerLabel[pixelLabelId] = RunLengthCode();

    // Initialise a vector of Runs for each row in the image
    for (unsigned y = 0; y < d_imageHeight; ++y)
      d_runsPerRowPerLabel[pixelLabelId].emplace_back();

    // Initialize blob container
    d_blobsDetectedPerLabel[pixelLabel] = vector<Blob>();
  }
}
