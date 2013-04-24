#include "visualcortex.ih"

void VisualCortex::streamDebugImage(cv::Mat cameraImage, std::shared_ptr<DataStreamer> streamer)
{
  // Only compose the image if at least one client is connected
  if (!streamer->hasImageClients())
    return;

  // Only provide an image every N cycles
  if (AgentState::getInstance().getTracker<CameraFrameState>()->updateCount() % d_streamFramePeriod != 0)
    return;

  Debugger& debugger = *d_debugger;
  auto t = Debugger::getTimestamp();

  auto lineDotColour = Colour::bgr(0, 0, 255);
  auto observedLineColour = Colour::bgr(255, 80, 80);
  auto expectedLineColour = Colour::bgr(0, 255, 0);
  auto horizonColour = Colour::bgr(0, 128, 255);

  Mat debugImage;

  if (d_imageType == ImageType::YCbCr)
  {
    debugImage = cameraImage;
  }
  else if (d_imageType == ImageType::RGB)
  {
    debugImage = cameraImage;
    PixelFilterChain chain;
    chain.pushFilter(&Colour::yCbCrToBgrInPlace);
    chain.applyFilters(debugImage);
  }
  else if (d_imageType == ImageType::Cartoon)
  {
    debugImage = d_cartoonPass->mat();
  }
  else if (d_imageType == ImageType::None)
  {
    debugImage = Mat(cameraImage.size(), cameraImage.type(), Scalar(0));
  }
  else
  {
    cout << "[VisualCortex::streamDebugging] Unknown image type requested!" << endl;
  }

  // Draw observed lines
  auto const& observedLineSegments = AgentState::get<CameraFrameState>()->getObservedLineSegments();
  if (d_shouldDrawObservedLines && observedLineSegments.size() > 0)
  {
    for (LineSegment2i const& line : observedLineSegments)
    {
      line.draw(debugImage, observedLineColour, 2);
    }
  }

  // Draw line dots
  if (d_shouldDrawLineDots && d_lineDotPass->lineDots.size() > 0)
  {
    for (auto const& lineDot : d_lineDotPass->lineDots)
    {
      debugImage.at<Colour::bgr>(lineDot.y(), lineDot.x()) = lineDotColour;
    }
  }

  // Draw blobs
  if (d_shouldDrawBlobs)
  {
    for (BlobType const& blobType : d_blobDetectPass->blobTypes())
    {
      auto blobColor = blobType.pixelLabel->hsvRange().toBgr()/*.invert()*/.toScalar();
      auto detectedBlobs = d_blobDetectPass->getDetectedBlobs().at(blobType.pixelLabel);
      for (Blob const& blob : detectedBlobs)
      {
        cv::rectangle(debugImage, blob.toRect(), blobColor);
      }
    }
  }

  // Draw expected lines
  if (d_shouldDrawExpectedLines)
  {
    Affine3d const& worldToAgent = AgentState::get<WorldFrameState>()->getPosition().agentWorldTransform();
    Affine3d const& agentToCamera = AgentState::get<BodyState>()->getLimb("camera")->transform.inverse();

    Affine3d const& worldToCamera = agentToCamera * worldToAgent;

    for (LineSegment3d const& line : d_fieldMap->getFieldLines())
    {
      auto p1 = d_cameraModel->pixelForDirection(worldToCamera * line.p1());
      auto p2 = d_cameraModel->pixelForDirection(worldToCamera * line.p2());

      if (p1.hasValue() && p2.hasValue())
      {
        LineSegment2i line2i(*p1.value(), *p2.value());

        line2i.draw(debugImage, expectedLineColour, 1);
      }
    }
  }

  // Draw horizon
  if (d_shouldDrawHorizon)
  {
    auto const& body = AgentState::get<BodyState>();
    auto neckJoint = body->getLimb("neck")->joints[0];
    Affine3d cameraToFootRotation = body->getCameraAgentTransform();

    Vector2i p1(0,0);
    p1.y() = d_spatialiser->findHorizonForColumn(p1.x(), cameraToFootRotation);

    Vector2i p2(d_cameraModel->imageWidth() - 1, 0);
    p2.y() = d_spatialiser->findHorizonForColumn(p2.x(), cameraToFootRotation);

    LineSegment2i line2i(p1, p2);

    line2i.draw(debugImage, horizonColour, 1);
  }

  streamer->streamImage(debugImage);
  t = debugger.timeEvent(t, "Debug Image Streaming");
}
