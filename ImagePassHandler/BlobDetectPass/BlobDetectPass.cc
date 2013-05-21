#include "blobdetectpass.ih"

BlobDetectPass::BlobDetectPass(ushort imageWidth, ushort imageHeight, std::vector<shared_ptr<PixelLabel>> const& pixelLabels)
  : d_imageHeight(imageHeight),
    d_imageWidth(imageWidth),
    d_pixelLabels(pixelLabels),
    d_runsPerRowPerLabel(),
    d_currentRun(0, 0)
{
  // Create a run length code for each label
  for (auto const& pixelLabel : pixelLabels)
  {
    uchar pixelLabelId = pixelLabel->id();

    // A RunLengthCode is a vector of vectors of runs
    d_runsPerRowPerLabel[pixelLabelId] = RunLengthCode();

    // Initialise a vector of Runs for each row in the image
    for (unsigned y = 0; y < d_imageHeight; ++y)
      d_runsPerRowPerLabel[pixelLabelId].push_back(std::vector<bold::Run>());

    // Initialize blob container
    d_blobsDetectedPerLabel[pixelLabel] = vector<Blob>();
  }
}
