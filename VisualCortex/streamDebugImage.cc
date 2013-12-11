#include "visualcortex.ih"

void VisualCortex::streamDebugImage(cv::Mat cameraImage, shared_ptr<DataStreamer> streamer, SequentialTimer& t)
{
  // Only compose the image if at least one client is connected
  if (!streamer->hasCameraClients())
    return;

  // Only provide an image every N cycles
  if (AgentState::getInstance().getTracker<CameraFrameState>()->updateCount() % d_streamFramePeriod->getValue() != 0)
    return;

  auto lineDotColour = d_lineDotColour->getValue();
  auto observedLineColour = d_observedLineColour->getValue();
  auto expectedLineColour = d_expectedLineColour->getValue();
  auto horizonColour = d_horizonColour->getValue();
  auto fieldEdgeColour = d_fieldEdgeColour->getValue();

  Mat debugImage;

  ImageType imageType = d_imageType->getValue();

  // Select the base imagery
  switch (imageType)
  {
    case ImageType::YCbCr:
    {
      debugImage = cameraImage;
      break;
    }
    case ImageType::RGB:
    {
      debugImage = cameraImage;
      PixelFilterChain chain;
      chain.pushFilter(&Colour::yCbCrToBgrInPlace);
      chain.applyFilters(debugImage);
      break;
    }
    case ImageType::Cartoon:
    {
      debugImage = d_cartoonPass->mat();
      break;
    }
    case ImageType::None:
    {
      debugImage = Mat(cameraImage.size(), cameraImage.type(), Scalar(0));
      break;
    }
    default:
    {
      cerr << ccolor::error << "[VisualCortex::streamDebugging] Unknown image type requested!" << ccolor::reset << endl;
      break;
    }
  }

  // Draw observed lines
  auto const& observedLineSegments = AgentState::get<CameraFrameState>()->getObservedLineSegments();
  if (d_shouldDrawObservedLines->getValue() && observedLineSegments.size() > 0)
  {
    for (LineSegment2i const& line : observedLineSegments)
    {
      line.draw(debugImage, observedLineColour, 2);
    }
  }

  // Draw line dots
  if (d_shouldDetectLines->getValue() && d_shouldDrawLineDots->getValue() && d_lineDotPass->lineDots.size() > 0)
  {
    for (auto const& lineDot : d_lineDotPass->lineDots)
    {
      debugImage.at<Colour::bgr>(lineDot.y(), lineDot.x()) = lineDotColour;
    }
  }

  // Draw blobs
  if (d_shouldDrawBlobs->getValue())
  {
    for (auto const& pixelLabel : d_blobDetectPass->pixelLabels())
    {
      auto blobColorBgr = pixelLabel->hsvRange().toBgr();
      switch (imageType)
      {
        case ImageType::Cartoon:
        case ImageType::RGB:
          blobColorBgr = blobColorBgr.invert();
          break;
        default:
          break;
      }

      auto blobColor = blobColorBgr.toScalar();
      auto detectedBlobs = d_blobDetectPass->getDetectedBlobs().at(pixelLabel);
      for (Blob const& blob : detectedBlobs)
      {
        // Blobs with zero area were merged into other blobs
        if (blob.area != 0)
          cv::rectangle(debugImage, blob.toRect(), blobColor);
      }
    }
  }

  // Draw observed objects (ie. actual ball/goal posts chosen from blobs)
  if (d_shouldDrawObservedObjects->getValue())
  {
    auto cameraFrame = AgentState::get<CameraFrameState>();

    auto ball = cameraFrame->getBallObservation();
    if (ball)
    {
      auto ballColor = Colour::bgr(0, 0, 255);
      Rect rect((int)round(ball->x()), (int)round(ball->y()), 5, 5);
      cv::rectangle(debugImage, rect, ballColor.toScalar());
    }

    auto goals = cameraFrame->getGoalObservations();
    for (auto goal : goals)
    {
      auto goalColor = Colour::bgr(0, 255, 255);
      Rect rect((int)round(goal.x()), (int)round(goal.y()), 5, 5);
      cv::rectangle(debugImage, rect, goalColor.toScalar(), CV_FILLED);
    }
  }

  auto bodyState = AgentState::get<BodyState>();

  // Draw expected lines
  if (bodyState && d_shouldDrawExpectedLines->getValue())
  {
    Affine3d const& agentWorld = AgentState::get<WorldFrameState>()->getPosition().agentWorldTransform();
    Affine3d const& cameraAgent = bodyState->getCameraAgentTransform();

    Affine3d const& cameraWorld = cameraAgent * agentWorld;

    auto max = Vector2i(Config::getStaticValue<int>("camera.image-width"),
                        Config::getStaticValue<int>("camera.image-height"));

    for (LineSegment3d const& line : d_fieldMap->getFieldLines())
    {
      // TODO this degrades when lines start/end outside of the camera's FOV
      auto p1 = d_cameraModel->pixelForDirection(cameraWorld * line.p1());
      auto p2 = d_cameraModel->pixelForDirection(cameraWorld * line.p2());

      if (p1.hasValue() && p2.hasValue())
      {
        auto p1v = p1->cast<int>();
        auto p2v = p2->cast<int>();
        if (p1v != p2v)
        {
          LineSegment2i line2i(max - p1v, max - p2v);

          line2i.draw(debugImage, expectedLineColour, 1);
        }
      }
    }
  }

  // Draw horizon
  if (bodyState && d_shouldDrawHorizon->getValue())
  {
    auto neckJoint = bodyState->getLimb("neck")->joints[0];
    Affine3d cameraToFootRotation = bodyState->getCameraAgentTransform();

    Vector2i p1(0,0);
    p1.y() = d_spatialiser->findHorizonForColumn(p1.x(), cameraToFootRotation);

    Vector2i p2(d_cameraModel->imageWidth() - 1, 0);
    p2.y() = d_spatialiser->findHorizonForColumn(p2.x(), cameraToFootRotation);

    LineSegment2i line2i(p1, p2);

    line2i.draw(debugImage, horizonColour, 1);
  }

  // Draw field edge
  if (d_shouldDrawFieldEdge->getValue())
  {
    for (unsigned x = 0; x < debugImage.size().width; ++x)
    {
      uchar y = d_fieldEdgePass->getEdgeYValue(x);

      debugImage.at<Colour::bgr>(y, x) = fieldEdgeColour;
    }
  }

  streamer->streamImage(debugImage);
  t.timeEvent("Debug Image Streaming");
}
