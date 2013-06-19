#include "visualcortex.ih"

void VisualCortex::integrateImage(Mat& image, SequentialTimer& t)
{
  auto cameraFrame = AgentState::get<CameraFrameState>();

  //
  // PROCESS THE IMAGE
  //

  // Label the image;
  if (d_labelledImage.rows != image.rows || d_labelledImage.cols != image.cols)
    d_labelledImage = Mat(image.rows, image.cols, CV_8UC1);

  d_imageLabeller->label(image, d_labelledImage, d_shouldIgnoreAboveHorizon);
  t.timeEvent("Image Processing/Pixel Label");

  // Perform the image pass
  d_imagePassRunner->pass(d_labelledImage);
  t.timeEvent("Image Processing/Pass");

  // Find lines

  vector<LineSegment2i> observedLineSegments;
  if (d_shouldDetectLines)
  {
    observedLineSegments = d_lineFinder->findLineSegments(d_lineDotPass->lineDots);
    t.timeEvent("Image Processing/Line Search");
  }

  // Find blobs
  auto blobsPerLabel = d_blobDetectPass->detectBlobs();
  t.timeEvent("Image Processing/Blob Search");

  //
  // UPDATE STATE
  //

  vector<Vector2d> goalPositions;
  Maybe<Vector2d> ballPosition = Maybe<Vector2d>::empty();

  // Do we have a ball?
  if (blobsPerLabel[d_ballLabel].size() > 0)
  {
    // The first is the biggest, topmost ball blob
    for (Blob const& ballBlob : blobsPerLabel[d_ballLabel])
    {
      // Ignore balls that are too small (avoid noise)
      if (ballBlob.area < d_minBallArea)
        continue;
      
      // Ignore ball if it appears outside the field edge
      //
      // Is not the ball if the bottom right corner of the ball is within the field edge determined by the bottom right corner
      if (ballBlob.ul.y() > d_fieldEdgePass->getEdgeYValue(ballBlob.ul.x()))
        continue;

      Vector2d pos = ballBlob.mean;

      // TODO discard blobs that would be too large/small for the ball we expect at that position of the frame
//       d_cameraModel->directionForPixel(pos);
      bool isCorrectSizeForPosition = true;

      if (isCorrectSizeForPosition)
      {
        // TODO take the curvature of the ball into account -- project middle of blob on the plane z=ballRadius
        // Take the bottom of the ball as observation
        pos.y() = ballBlob.ul.y();
        ballPosition = Maybe<Vector2d>(pos);
      }
    }
  }

  // Do we have goal posts?
  for (Blob const& goalBlob : blobsPerLabel[d_goalLabel])
  {
    // Ignore goal if it appears outside of field
    //
    // NOTE Process this before anything else as anything above the field edge is wasting our time
    int allowedGoalFieldEdgeErrorPixels = 5;
    if (goalBlob.ul.y() > d_fieldEdgePass->getEdgeYValue(goalBlob.ul.x()) + allowedGoalFieldEdgeErrorPixels)
        continue;
    
    // TODO apply this filtering earlier, so that the debug image doesn't show unused goal blobs
    Vector2i wh = goalBlob.br - goalBlob.ul;
    
    if (wh.minCoeff() > 5 &&  // Ignore small blobs
        wh.y() > wh.x())      // Taller than it is wide
    {
      Run const& topRun = *goalBlob.runs.begin();

      // Take center of topmost run (the first)
      Vector2d pos(
        (topRun.endX + topRun.startX) / 2.0f,
        topRun.y
      );

      goalPositions.push_back(pos);
    }
  }

  AgentState::getInstance().set(make_shared<CameraFrameState const>(ballPosition, goalPositions, observedLineSegments));

  t.timeEvent("Image Processing/Updating State");
}
