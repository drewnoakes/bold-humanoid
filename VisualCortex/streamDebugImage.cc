#include "visualcortex.ih"

void VisualCortex::streamDebugImage(cv::Mat cameraImage, DataStreamer* streamer)
{
  if (!streamer->shouldProvideImage())
    return;

  auto& debugger = Debugger::getInstance();
  auto t = Debugger::getTimestamp();

  ImageType imageType = streamer->getImageType();

  Mat debugImage;

  if (imageType == ImageType::YCbCr)
  {
    debugImage = cameraImage;
  }
  else if (imageType == ImageType::RGB)
  {
    debugImage = cameraImage;
    PixelFilterChain chain;
    chain.pushFilter([](unsigned char* pxl) {
      Colour::YCbCr* ycbcr = reinterpret_cast<Colour::YCbCr*>(pxl);
      Colour::bgr* bgr = reinterpret_cast<Colour::bgr*>(pxl);
      *bgr = (*ycbcr).toBgrInt();
    });
    chain.applyFilters(debugImage);
  }
  else if (imageType == ImageType::Cartoon)
  {
    debugImage = d_cartoonPass->mat();
  }
  else if (imageType == ImageType::None)
  {
    debugImage = Mat(cameraImage.size(), cameraImage.type(), Scalar(0));
  }
  else
  {
    cout << "[VisualCortex::streamDebugging] Unknown image type requested!" << endl;
  }

  // Draw lines
  if (streamer->drawLines() && d_lines.size() > 0)
  {
    for (auto const& hypothesis : d_lines)
    {
      auto line = hypothesis.toLine();
      cv::line(debugImage,
              cv::Point(hypothesis.min().x(), hypothesis.min().y()),
              cv::Point(hypothesis.max().x(), hypothesis.max().y()),
              Colour::bgr(255,0,0).toScalar(),
              2);
    }
  }

  // Draw blobs
  if (streamer->drawBlobs())
  {
    for (BlobType const& blobType : d_blobDetectPass->blobTypes())
    {
      auto blobColor = blobType.pixelLabel.hsvRange().toBgr().invert().toScalar();
      auto detectedBlobs = d_blobDetectPass->getDetectedBlobs().at(blobType.pixelLabel);
      for (Blob const& blob : detectedBlobs)
      {
	cv::rectangle(debugImage, blob.toRect(), blobColor);
      }
    }
  }
  
  streamer->streamImage(debugImage);
  t = debugger.timeEvent(t, "Debug Image Streaming");
}
