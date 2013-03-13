#include "blobdetectpass.ih"

BlobDetectPass::BlobDetectPass(int imageWidth, int imageHeight, std::vector<BlobType> const& blobTypes)
  : d_blobTypes(blobTypes),
    d_imageHeight(imageHeight),
    d_imageWidth(imageWidth),
    d_runsPerRowPerLabel(),
    d_currentRun(0, 0)
{
  // Create a run length code for each label
  for (BlobType const& blobType : blobTypes)
  {
    uchar pixelLabelId = blobType.pixelLabel.id();
    
    // A RunLengthCode is a vector of vectors of runs
    d_runsPerRowPerLabel[pixelLabelId] = RunLengthCode();
    
    // Initialise a vector of Runs for each row in the image
    for (unsigned y = 0; y < d_imageHeight; ++y)
      d_runsPerRowPerLabel[pixelLabelId].push_back(std::vector<bold::Run>());

    // Initialize blob container
    d_blobsDetectedPerLabel[blobType.pixelLabel] = vector<Blob>();
  }
}
