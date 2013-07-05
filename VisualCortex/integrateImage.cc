#include "visualcortex.ih"

void VisualCortex::integrateImage(Mat& image, SequentialTimer& t)
{
  //
  // Record frame, if required
  //
  static Clock::Timestamp lastRecordTime;
  if (d_recordNextFrame || (d_isRecordingFrames && Clock::getSecondsSince(lastRecordTime) > 1.0))
  {
    saveImage(image);
    lastRecordTime = Clock::getTimestamp();
    d_recordNextFrame = false;
    t.timeEvent("Saving Frame To File");
  }

  auto cameraFrame = AgentState::get<CameraFrameState>();

  // TODO why are SequentialTimer calls here prefixed with "Image Processing"? This is supposed to be taken care of via the enter/exit pattern

  //
  // PROCESS THE IMAGE
  //

  // Label the image
  if (d_labelledImage.rows != image.rows || d_labelledImage.cols != image.cols)
    d_labelledImage = Mat(image.rows, image.cols, CV_8UC1);

  // Produce an image of labelled pixels.
  // If the option is enabled, any pixels above the horizon will be set to zero.
  d_imageLabeller->label(image, d_labelledImage, d_shouldIgnoreAboveHorizon);
  t.timeEvent("Image Processing/Pixel Label");

  // Perform the image pass
  d_imagePassRunner->pass(d_labelledImage);
  t.timeEvent("Image Processing/Pass");

  if (d_shouldCountLabels)
  {
    AgentState::getInstance().set(make_shared<LabelCountState const>(d_labelCountPass->getCounts()));
    t.timeEvent("Image Processing/Store Label Count");
  }

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

  // Might we have a ball?
  if (blobsPerLabel[d_ballLabel].size() > 0)
  {
    // Merge ball blobs
    vector<Blob>& ballBlobs = blobsPerLabel[d_ballLabel];
    for (int i = 0; i < min(10, (int)ballBlobs.size()); ++i)
    {
      Blob& larger = ballBlobs[i];

      if (larger.area == 0)
        continue;

      if (larger.area < d_minBallArea)
      {
        // Blobs are sorted, largest first, so if this is too small, the rest will be too
        break;
      }

      for (int j = i + 1; j < min(10, (int)ballBlobs.size()); ++j)
      {
        Blob& smaller = ballBlobs[j];

        if (smaller.area == 0)
          continue;

        if (shouldMergeBallBlobs(larger.bounds(), smaller.bounds()))
        {
          larger.merge(smaller);
          // Indicate that the smaller one is no longer in use
          smaller.area = 0;
        }
      }
    }
    t.timeEvent("Image Processing/Ball Blob Merging");

    // The first is the biggest, topmost ball blob
    for (Blob const& ballBlob : ballBlobs)
    {
      if (ballBlob.area == 0)
      {
        // Ignore blobs that were previously merged into another blob (zero area)
        continue;
      }

      // Ignore balls that are too small (avoid noise)
      if (ballBlob.area < d_minBallArea)
      {
        // As blobs are sorted largest to smallest, stop at the first one that's too small
        break;
      }

      // Ignore ball if it appears outside the field edge
      //
      if (ballBlob.ul.y() > d_fieldEdgePass->getEdgeYValue(ballBlob.ul.x()))
      // This blob can not be the ball if its upper left corner is below the field edge.
      // Remember that the image appears upside down.
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
        break;
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

    if (wh.minCoeff() > d_minGoalDimensionPixels &&  // Ignore small blobs
        wh.y() > wh.x())                             // Taller than it is wide
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
