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
    chain.pushFilter(&Colour::yCbCrToBgbInPlace);
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

  // Draw observed lines
  if (streamer->drawObservedLines() && d_lines.size() > 0)
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

  // Draw line dots
  if (streamer->drawLineDots() && d_lineDotPass->lineDots.size() > 0)
  {
    auto colour = Colour::bgr(0,0,255);
    for (auto const& lineDot : d_lineDotPass->lineDots)
    {
      debugImage.at<Colour::bgr>(lineDot.y(), lineDot.x()) = colour;
    }
  }

  // Draw blobs
  if (streamer->drawBlobs())
  {
    for (BlobType const& blobType : d_blobDetectPass->blobTypes())
    {
      auto blobColor = blobType.pixelLabel.hsvRange().toBgr()/*.invert()*/.toScalar();
      auto detectedBlobs = d_blobDetectPass->getDetectedBlobs().at(blobType.pixelLabel);
      for (Blob const& blob : detectedBlobs)
      {
        cv::rectangle(debugImage, blob.toRect(), blobColor);
      }
    }
  }

  // Draw expected lines
  if (streamer->drawExpectedLines())
  {
    auto fieldLines = WorldModel::getInstance().getFieldLines();
    Projector projector = AgentModel::getInstance().getCameraModel().getProjector();

    for (LineSegment2d const& line : fieldLines)
    {
      LineSegment3d line3d = line.to3();

      Vector2i p1 = projector(line3d.p1());
      Vector2i p2 = projector(line3d.p2());

      cv::line(debugImage,
              cv::Point(p1.x(), p1.y()),
              cv::Point(p2.x(), p2.y()),
              Colour::bgr(0,255,0).toScalar(),
              2);
    }
  }

  streamer->streamImage(debugImage);
  t = debugger.timeEvent(t, "Debug Image Streaming");
}
