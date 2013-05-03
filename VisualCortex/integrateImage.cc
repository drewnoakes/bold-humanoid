#include "visualcortex.ih"

void VisualCortex::integrateImage(Mat& image)
{
  Debugger& debugger = *d_debugger;
  auto t = Clock::getTimestamp();

  auto cameraFrame = AgentState::get<CameraFrameState>();

  //
  // PROCESS THE IMAGE
  //

  // Label the image;
  if (d_labelledImage.rows != image.rows || d_labelledImage.cols != image.cols)
    d_labelledImage = Mat(image.rows, image.cols, CV_8UC1);

  d_imageLabeller->label(image, d_labelledImage, true);
  t = debugger.timeEvent(t, "Image Processing/Pixel Label");

  // Perform the image pass
  d_imagePassRunner->pass(d_labelledImage);
  t = debugger.timeEvent(t, "Image Processing/Pass");

  // Find lines

  vector<LineSegment2i> observedLineSegments;
  if (d_detectLines)
  {
    observedLineSegments = d_lineFinder->findLineSegments(d_lineDotPass->lineDots);
    t = debugger.timeEvent(t, "Image Processing/Line Search");
  }

  // Find blobs
  auto blobsPerLabel = d_blobDetectPass->detectBlobs();
  t = debugger.timeEvent(t, "Image Processing/Blob Search");

  //
  // UPDATE STATE
  //

  vector<Vector2f> goalPositions;
  Maybe<Vector2f> ballPosition = Maybe<Vector2f>::empty();

  // Do we have a ball?
  if (blobsPerLabel[d_ballLabel].size() > 0)
  {
    // The first is the biggest, topmost ball blob
    for (Blob const& ballBlob : blobsPerLabel[d_ballLabel])
    {
      if (ballBlob.area < d_minBallArea)
        break;

      Vector2f pos = ballBlob.mean;

      // TODO discard blobs that would be too large/small for the ball we expect at that position of the frame
//       d_cameraModel->directionForPixel(pos);
      bool isCorrectSizeForPosition = true;

      if (isCorrectSizeForPosition)
      {
        // TODO take the curvature of the ball into account -- project middle of blob on the plane z=ballRadius
        // Take the bottom of the ball as observation
        pos.y() = ballBlob.ul.y();
        ballPosition = Maybe<Vector2f>(pos);
      }
    }
  }

  // Do we have goal posts?
  for (Blob const& goalBlob : blobsPerLabel[d_goalLabel])
  {
    // TODO apply this filtering earlier, so that the debug image doesn't show unused goal blobs
    Vector2i wh = goalBlob.br - goalBlob.ul;
    if (wh.minCoeff() > 5 &&  // Ignore small blobs
        wh.y() > wh.x())      // Taller than it is wide
    {
      Run const& topRun = *goalBlob.runs.begin();

      // Take center of topmost run (the first)
      Vector2f pos(
        (topRun.endX + topRun.startX) / 2.0f,
        topRun.y
      );

      goalPositions.push_back(pos);
    }
  }

  AgentState::getInstance().set(make_shared<CameraFrameState const>(ballPosition, goalPositions, observedLineSegments));

  debugger.timeEvent(t, "Image Processing/Updating State");
}
