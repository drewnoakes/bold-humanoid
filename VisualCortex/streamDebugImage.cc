#include "visualcortex.ih"

void VisualCortex::streamDebugImage(cv::Mat cameraImage, std::shared_ptr<DataStreamer> streamer)
{
  if (!streamer->shouldProvideImage())
    return;

  auto& debugger = Debugger::getInstance();
  auto t = Debugger::getTimestamp();

  ImageType imageType = streamer->getImageType();

  auto lineDotColour = Colour::bgr(0, 0, 255);
  auto observedLineColour = Colour::bgr(255, 80, 80);
  auto expectedLineColour = Colour::bgr(0, 255, 0);

  Mat debugImage;

  if (imageType == ImageType::YCbCr)
  {
    debugImage = cameraImage;
  }
  else if (imageType == ImageType::RGB)
  {
    debugImage = cameraImage;
    PixelFilterChain chain;
    chain.pushFilter(&Colour::yCbCrToBgrInPlace);
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
  if (streamer->shouldDrawObservedLines() && d_observedLineSegments.size() > 0)
  {
    for (LineSegment2i const& line : d_observedLineSegments)
    {
      line.draw(debugImage, observedLineColour, 2);
    }
  }

  // Draw line dots
  if (streamer->shouldDrawLineDots() && d_lineDotPass->lineDots.size() > 0)
  {
    for (auto const& lineDot : d_lineDotPass->lineDots)
    {
      debugImage.at<Colour::bgr>(lineDot.y(), lineDot.x()) = lineDotColour;
    }
  }

  // Draw blobs
  if (streamer->shouldDrawBlobs())
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
  if (streamer->shouldDrawExpectedLines())
  {
    auto fieldLines = WorldModel::getInstance().getFieldLines();
    Projector projector = AgentModel::getInstance().getCameraModel()->getProjector();

    double torsoX = 1.0;
    double torsoY = -1.0;
    double torsoTheta = 0;

    // TODO calculate correct camera position, using neck angles, agent height, camera position within head
    double cameraHeight = 0.45;

    Affine3d worldToCamera(Translation3d(0,0,10)); // * AngleAxisf(a,axis) * Scaling(s);
//     Affine3d worldToCamera(AngleAxisd(M_PI/2, Vector3d::UnitY));
//     worldToCamera.rotate();

    for (LineSegment2d const& line : fieldLines)
    {
      LineSegment3d line3d = line.to<3>();

      Vector2i p1 = projector(worldToCamera * line3d.p1());
      Vector2i p2 = projector(worldToCamera * line3d.p2());

      LineSegment2i line2i(p1, p2);

      line2i.draw(debugImage, expectedLineColour, 1);
    }
  }

  streamer->streamImage(debugImage);
  t = debugger.timeEvent(t, "Debug Image Streaming");
}
