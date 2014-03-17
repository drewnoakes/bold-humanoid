#include "visualcortex.ih"

void VisualCortex::streamDebugImage(cv::Mat cameraImage, SequentialTimer& t)
{
  // Only compose the image if at least one client is connected
  if (!d_dataStreamer->hasCameraClients())
    return;

  // Only provide an image every N cycles
  if (State::getTracker<CameraFrameState>()->updateCount() % d_streamFramePeriod->getValue() != 0)
    return;

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
      debugImage = getHandler<CartoonPass>()->mat().clone();
      break;
    }
    case ImageType::None:
    {
      debugImage = Mat(cameraImage.size(), cameraImage.type(), Scalar(0));
      break;
    }
    default:
    {
      log::error("VisualCortex::streamDebugging") << "Unknown image type requested!";
      break;
    }
  }

  // Draw observed lines
  auto const& observedLineSegments = State::get<CameraFrameState>()->getObservedLineSegments();
  if (d_shouldDrawObservedLines->getValue() && observedLineSegments.size() > 0)
  {
    auto observedLineColour = d_observedLineColour->getValue();
    for (LineSegment2i const& line : observedLineSegments)
      draw(line, debugImage, observedLineColour, 2);
  }

  // Draw line dots
  if (d_shouldDetectLines->getValue() && d_shouldDrawLineDots->getValue() && getHandler<LineDotPass<uchar>>()->lineDots.size() > 0)
  {
    auto lineDotColour = d_lineDotColour->getValue();
    for (auto const& lineDot : getHandler<LineDotPass<uchar>>()->lineDots)
      debugImage.at<Colour::bgr>(lineDot.y(), lineDot.x()) = lineDotColour;
  }

  // Draw blobs
  if (d_shouldDrawBlobs->getValue())
  {
    for (auto const& pixelLabel : getHandler<BlobDetectPass>()->pixelLabels())
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
      auto detectedBlobs = getHandler<BlobDetectPass>()->getDetectedBlobs().at(pixelLabel);
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
    auto cameraFrame = State::get<CameraFrameState>();

    auto ball = cameraFrame->getBallObservation();
    if (ball)
    {
      static auto ballColor = Colour::bgr(0, 0, 255).toScalar();
      Rect rect((int)round(ball->x()), (int)round(ball->y()), 5, 5);
      cv::rectangle(debugImage, rect, ballColor, CV_FILLED);
    }

    auto goals = cameraFrame->getGoalObservations();
    for (auto goal : goals)
    {
      static auto goalColor = Colour::bgr(0, 255, 255).toScalar();
      Rect rect((int)round(goal.x()), (int)round(goal.y()), 5, 5);
      cv::rectangle(debugImage, rect, goalColor, CV_FILLED);
    }
  }

  auto bodyState = State::get<BodyState>(StateTime::CameraImage);

  // Draw expected lines
  bool drawExpectedLines = d_shouldDrawExpectedLines->getValue();
  bool drawExpectedLineEdges = d_shouldDrawExpectedLineEdges->getValue();
  if (drawExpectedLines || drawExpectedLineEdges)
  {
    auto const& worldFrameState = State::get<WorldFrameState>();
    Affine3d const& agentWorld = worldFrameState->getPosition().agentWorldTransform();
    Affine3d const& cameraAgent = bodyState->getCameraAgentTransform();

    Affine3d const& cameraWorld = cameraAgent * agentWorld;

    auto max = Vector2i(Config::getStaticValue<int>("camera.image-width"),
                        Config::getStaticValue<int>("camera.image-height"));

    auto expectedLineColour = d_expectedLineColour->getValue();

    auto visibleFieldPoly = worldFrameState->getVisibleFieldPoly();

    if (visibleFieldPoly.hasValue())
    {
      for (LineSegment3d const& expectedLine : drawExpectedLineEdges ? d_fieldMap->getFieldLineEdges() : d_fieldMap->getFieldLines())
      {
        // Clip world lines based upon visible field poly before transforming to camera frame
        Maybe<LineSegment2d> clippedLine2 = visibleFieldPoly->clipLine(expectedLine.to<2>());
        if (clippedLine2.hasValue())
        {
          LineSegment3d clippedLine = clippedLine2.value().to<3>();

          auto p1 = d_cameraModel->pixelForDirection(cameraWorld * clippedLine.p1());
          auto p2 = d_cameraModel->pixelForDirection(cameraWorld * clippedLine.p2());

          if (p1.hasValue() && p2.hasValue())
          {
            auto p1v = p1->cast<int>();
            auto p2v = p2->cast<int>();
            if (p1v != p2v)
            {
              LineSegment2i line2i(max - p1v, max - p2v);

              draw(line2i, debugImage, expectedLineColour, 1);
            }
          }
        }
      }
    }
  }

  // Draw horizon
  if (d_shouldDrawHorizon->getValue())
  {
    auto neckJoint = bodyState->getLimb("neck")->joints[0];
    Affine3d const& cameraAgentTr = bodyState->getCameraAgentTransform();

    Vector2i p1(0,0);
    p1.y() = d_spatialiser->findHorizonForColumn(p1.x(), cameraAgentTr);

    Vector2i p2(d_cameraModel->imageWidth() - 1, 0);
    p2.y() = d_spatialiser->findHorizonForColumn(p2.x(), cameraAgentTr);

    LineSegment2i line2i(p1, p2);

    draw(line2i, debugImage, d_horizonColour->getValue(), 1);
  }

  // Draw occlusion edge
  if (d_shouldDrawOcclusionEdge->getValue())
  {
    auto occlusionColour = d_occlusionEdgeColour->getValue().toScalar();
    auto rays = d_fieldEdgePass->getOcclusionRays();
    auto lastPoint = rays[0].first;
    for (auto const& ray : rays)
    {
      // Connected left-to-right lines
      cv::line(debugImage,
               Point(lastPoint.x(), lastPoint.y()),
               Point(ray.first.x(), ray.first.y()),
               occlusionColour);

      // Lines from front to back
      cv::line(debugImage,
               Point(ray.first.x(), ray.first.y()),
               Point(ray.second.x(), ray.second.y()),
               occlusionColour);

      lastPoint = ray.first;
    }
  }

  // Draw field edge
  if (d_shouldDrawFieldEdge->getValue())
  {
    auto fieldEdgeColour = d_fieldEdgeColour->getValue();
    for (ushort x = 0; x < debugImage.size().width; ++x)
    {
      ushort y = d_fieldEdgePass->getEdgeYValue(x);

      debugImage.at<Colour::bgr>(y, x) = fieldEdgeColour;
    }
  }

  // Draw calibration lines
  if (d_shouldDrawCalibration->getValue())
  {
    auto calibrationColour = d_calibrationColour->getValue().toScalar();

    int w = d_cameraModel->imageWidth();
    int h = d_cameraModel->imageHeight();
    int midX = round(w / 2.0);
    int midY = round(h / 2.0);
    cv::line(debugImage, Point(0, midY), Point(w-1, midY), calibrationColour);
    cv::line(debugImage, Point(midX, 0), Point(midX, h-1), calibrationColour);
  }

  auto imageEncoding = d_imageType->getValue() == ImageType::None || d_imageType->getValue() == ImageType::Cartoon
    ? ".png"
    : ".jpg";

  d_dataStreamer->streamImage(debugImage, imageEncoding);
  t.timeEvent("Stream Debug Image");

  if (d_saveNextDebugFrame)
  {
    d_saveNextDebugFrame = false;
    saveImage(debugImage);
    t.timeEvent("Save Debug Image");
  }
}
