#include "visualcortex.ih"

using namespace bold::Colour;

void VisualCortex::streamDebugImage(cv::Mat cameraImage, SequentialTimer& t)
{
  // Only compose the image if at least one client is connected
  if (!d_dataStreamer->hasCameraClients())
    return;

  // Only provide an image every N cycles
  if (State::getTracker<CameraFrameState>()->updateCount() % d_streamFramePeriod->getValue() != 0)
    return;

  auto const& cameraFrame = State::get<CameraFrameState>();

  Mat debugImage;

  ImageType imageType = d_imageType->getValue();

  bool drawDebugData = true;

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
      chain.pushFilter(&yCbCrToBgrInPlace);
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
    case ImageType::Teacher:
    {
      drawDebugData = false;

      if (d_labelTeacher->requestedLabel())
      {
        debugImage = d_labelTeacher->label(d_labelTeacher->getLabelToTrain());
      }
      else
      {
        auto trainImage = d_labelTeacher->getBGRTrainImage();
        if (!(trainImage.rows == cameraImage.rows && trainImage.cols == cameraImage.cols))
        {
          debugImage = cameraImage;
          PixelFilterChain chain;
          chain.pushFilter(&Colour::yCbCrToBgrInPlace);
          chain.applyFilters(debugImage);
        }
        else
        {
          auto maskImage = d_labelTeacher->getMask();
          if (maskImage.rows == trainImage.rows && maskImage.cols == trainImage.cols)
          {
            cv::Mat multiChannelMask;
            cv::merge(vector<cv::Mat>{maskImage, maskImage, maskImage}, multiChannelMask);
            cv::bitwise_xor(trainImage, multiChannelMask, debugImage);
          }
          else
            debugImage = trainImage;

          cv::rectangle(debugImage, cv::Rect(0, 0, debugImage.cols, debugImage.rows), cv::Scalar{0, 0, 255});
        }
      }
    }
    default:
    {
      log::error("VisualCortex::streamDebugging") << "Unknown image type requested!";
      break;
    }
  }

  // Draw observed lines
  auto const& observedLineSegments = State::get<CameraFrameState>()->getObservedLineSegments();
  if (drawDebugData && d_shouldDrawObservedLines->getValue() && observedLineSegments.size() > 0)
  {
    auto observedLineColour = d_observedLineColour->getValue();
    for (LineSegment2i const& line : observedLineSegments)
      Painter::draw(line, debugImage, observedLineColour, 2);
  }

  // Draw line dots
  if (drawDebugData && d_shouldDetectLines->getValue() && d_shouldDrawLineDots->getValue() && getHandler<LineDotPass<uchar>>()->lineDots.size() > 0)
  {
    auto lineDotColour = d_lineDotColour->getValue();
    for (auto const& lineDot : getHandler<LineDotPass<uchar>>()->lineDots)
      debugImage.at<bgr>(lineDot.y(), lineDot.x()) = lineDotColour;
  }

  // Draw blobs
  if (drawDebugData && d_shouldDrawBlobs->getValue())
  {
    auto const& blobsByLabel = getHandler<BlobDetectPass>()->getDetectedBlobs();

    for (auto const& pixelLabel : getHandler<BlobDetectPass>()->pixelLabels())
    {
      // Determine outline colour for the blob
      cv::Scalar blobColour;
      if (pixelLabel->getName() == "Goal")
        blobColour = bgr::yellow.toScalar();
      else if (pixelLabel->getName() == "Ball")
        blobColour = bgr::red.toScalar();
      else if (pixelLabel->getName() == "Cyan")
        blobColour = bgr::cyan.toScalar();
      else if (pixelLabel->getName() == "Magenta")
        blobColour = bgr::magenta.toScalar();
      else
        blobColour = pixelLabel->modalColour().toBgr().toScalar();

      for (auto const& blob : blobsByLabel.at(pixelLabel))
      {
        // Blobs with zero area were merged into other blobs
        if (blob.area != 0)
          cv::rectangle(debugImage, blob.toRect(), blobColour);
      }
    }
  }

  // Draw observed objects (ie. actual ball/goal posts chosen from blobs)
  if (drawDebugData && d_shouldDrawObservedObjects->getValue())
  {
    auto ball = cameraFrame->getBallObservation();
    if (ball)
    {
      static auto ballColor = bgr(0, 0, 255).toScalar();
      Rect rect((int)round(ball->x()), (int)round(ball->y()), 5, 5);
      cv::rectangle(debugImage, rect, ballColor, CV_FILLED);
    }

    auto goals = cameraFrame->getGoalObservations();
    for (auto goal : goals)
    {
      static auto goalColor = bgr(0, 255, 255).toScalar();
      Rect rect((int)round(goal.x()), (int)round(goal.y()), 5, 5);
      cv::rectangle(debugImage, rect, goalColor, CV_FILLED);
    }
  }

  auto bodyState = State::get<BodyState>(StateTime::CameraImage);

  // Draw expected lines
  bool drawExpectedLines = d_shouldDrawExpectedLines->getValue();
  bool drawExpectedLineEdges = d_shouldDrawExpectedLineEdges->getValue();
  if (drawDebugData && (drawExpectedLines || drawExpectedLineEdges))
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
      for (LineSegment3d const& expectedLine : drawExpectedLineEdges ? FieldMap::getFieldLineEdges() : FieldMap::getFieldLines())
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

              Painter::draw(line2i, debugImage, expectedLineColour, 1);
            }
          }
        }
      }
    }
  }

  // Draw horizon
  if (drawDebugData && d_shouldDrawHorizon->getValue())
  {
    auto neckJoint = bodyState->getLimb("neck")->getLimb()->joints[0];
    Affine3d const& cameraAgentTr = bodyState->getCameraAgentTransform();

    Vector2i p1(0,0);
    p1.y() = d_spatialiser->findHorizonForColumn(p1.x(), cameraAgentTr);

    Vector2i p2(d_cameraModel->imageWidth() - 1, 0);
    p2.y() = d_spatialiser->findHorizonForColumn(p2.x(), cameraAgentTr);

    LineSegment2i line2i(p1, p2);

    Painter::draw(line2i, debugImage, d_horizonColour->getValue(), 1);
  }

  // Draw occlusion edge
  if (drawDebugData && d_shouldDrawOcclusionEdge->getValue())
  {
    auto occlusionColour = d_occlusionEdgeColour->getValue().toScalar();
    auto const& rays = cameraFrame->getOcclusionRays();
    auto lastPoint = rays[0].near();
    for (auto const& ray : rays)
    {
      // Connected left-to-right lines
      cv::line(debugImage,
               Point(lastPoint.x(), lastPoint.y()),
               Point(ray.near().x(), ray.near().y()),
               occlusionColour);

      // Lines from front to back
      cv::line(debugImage,
               Point(ray.near().x(), ray.near().y()),
               Point(ray.far().x(), ray.far().y()),
               occlusionColour);

      lastPoint = ray.near();
    }
  }

  // Draw field edge
  if (drawDebugData && d_shouldDrawFieldEdge->getValue())
  {
    auto fieldEdgeColour = d_fieldEdgeColour->getValue();
    for (ushort x = 0; x < debugImage.size().width; ++x)
    {
      ushort y = d_fieldEdgePass->getEdgeYValue(x);

      debugImage.at<bgr>(y, x) = fieldEdgeColour;
    }
  }

  // Draw calibration lines
  if (drawDebugData && d_shouldDrawCalibration->getValue())
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
