#include "visualcortex.ih"

void VisualCortex::integrateImage(cv::Mat& image)
{
  auto& debugger = Debugger::getInstance();
  auto t = Debugger::getTimestamp();
  shared_ptr<CameraFrameState> cameraFrame = AgentState::getInstance().cameraFrame();

  //
  // PROCESS THE IMAGE
  //

  // Label the image;
  if (d_labelledImage.rows != image.rows || d_labelledImage.cols != image.cols)
    d_labelledImage = cv::Mat(image.rows, image.cols, CV_8UC1);

  d_imageLabeller->label(image, d_labelledImage);
  t = debugger.timeEvent(t, "Image Processing/Pixel Label");

  // Perform the image pass
  d_imagePassRunner->pass(d_labelledImage);
  t = debugger.timeEvent(t, "Image Processing/Pass");

  // Find lines

  auto observedLineSegments = d_lineFinder->findLineSegments(d_lineDotPass->lineDots);
  t = debugger.timeEvent(t, "Image Processing/Line Search");

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
    Blob const& ball = *blobsPerLabel[d_ballLabel].begin();

    if (ball.area > d_minBallArea)
    {
      Vector2f pos = ball.mean;
      // Take the bottom of the ball as observation
      pos.y() = ball.ul.y();
      ballPosition = Maybe<Vector2f>(pos);
    }
  }

  // Do we have goal posts?
  for (Blob const& b : blobsPerLabel[d_goalLabel])
  {
    Vector2i wh = b.br - b.ul;
    if (wh.minCoeff() > 5  &&  // ignore small blobs
        wh.y() > wh.x())       // Higher than it is lower
    {
      Run const& topRun = *b.runs.begin();

      // Take center of topmost run (the first)
      Vector2f pos(
        (topRun.endX + topRun.startX) / 2.0f,
        topRun.y
      );

      goalPositions.push_back(pos);
    }
  }

  AgentState::getInstance().setCameraFrame(make_shared<CameraFrameState>(ballPosition, goalPositions, observedLineSegments));

  debugger.timeEvent(t, "Image Processing/Updating State");
}
