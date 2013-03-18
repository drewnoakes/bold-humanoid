#include "visualcortex.ih"

void VisualCortex::integrateImage(cv::Mat& image)
{
  auto& debugger = Debugger::getInstance();
  auto t = Debugger::getTimestamp();

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
  d_lines = d_lineFinder->findLineSegments(d_lineDotPass->lineDots);
  t = debugger.timeEvent(t, "Image Processing/Line Search");

  //
  // UPDATE STATE
  //

  auto blobsPerLabel = d_blobDetectPass->getDetectedBlobs();
  d_observations.clear();
  d_goalObservations.clear();

  // Do we have a ball?
  d_isBallVisible = false;
  if (blobsPerLabel[d_ballLabel].size() > 0)
  {
    // The first is the biggest, topmost ball blob
    Blob const& ball = *blobsPerLabel[d_ballLabel].begin();

    if (ball.area > d_minBallArea)
    {
      Observation ballObs;
      ballObs.type = O_BALL;
      ballObs.pos = ball.mean;

      d_observations.push_back(ballObs);
      d_ballObservation = ballObs;
      d_isBallVisible = true;
    }
  }

  // Do we have goal posts?
  for (Blob const& b : blobsPerLabel[d_goalLabel])
  {
    Vector2i wh = b.br - b.ul;
    if (wh.minCoeff() > 5  &&  // ignore small blobs
        wh.y() > wh.x())       // Higher than it is lower
    {
      // Take center of topmost run (the first)
      Observation postObs;
      postObs.type = O_GOAL_POST;
      Run const& topRun = *b.runs.begin();
      postObs.pos.y() = topRun.y;
      postObs.pos.x() = (topRun.endX + topRun.startX) / 2.0f;

      d_observations.push_back(postObs);
      d_goalObservations.push_back(postObs);
    }
  }

  debugger.timeEvent(t, "Image Processing/Updating State");
}
