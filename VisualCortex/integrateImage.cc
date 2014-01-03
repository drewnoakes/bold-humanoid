#include "visualcortex.ih"

void VisualCortex::integrateImage(Mat& image, SequentialTimer& t)
{
  //
  // Record frame, if required
  //
  static Clock::Timestamp lastRecordTime;
  if (d_recordNextFrame || (d_isRecordingFrames->getValue() && Clock::getSecondsSince(lastRecordTime) > 1.0))
  {
    saveImage(image);
    lastRecordTime = Clock::getTimestamp();
    d_recordNextFrame = false;
    t.timeEvent("Saving Frame To File");
  }

  //
  // PROCESS THE IMAGE
  //

  // Label the image
  if (d_labelledImage.rows != image.rows || d_labelledImage.cols != image.cols)
    d_labelledImage = Mat(image.rows, image.cols, CV_8UC1);

  // Produce an image of labelled pixels.
  // If the option is enabled, any pixels above the horizon will be set to zero.
  t.enter("Pixel Label");
  d_imageLabeller->label(image, d_labelledImage, t, d_granularityFunction, d_shouldIgnoreAboveHorizon->getValue());
  t.exit();

  // Perform the image pass
  //long processedPixelCount = d_imagePassRunner->pass(d_labelledImage, d_granularityFunction);
  long processedPixelCount = d_imagePassRunner->passWithHandlers(d_imagePassHandlers, d_labelledImage, d_granularityFunction);

  long totalPixelCount = d_labelledImage.rows * d_labelledImage.cols;
  t.timeEvent("Pass");

  if (d_shouldCountLabels->getValue())
  {
    AgentState::getInstance().set(make_shared<LabelCountState const>(getHandler<LabelCountPass>()->getCounts()));
    t.timeEvent("Store Label Count");
  }

  // Find lines
  vector<LineSegment2i> observedLineSegments;
  if (d_shouldDetectLines->getValue())
  {
    observedLineSegments = d_lineFinder->findLineSegments(getHandler<LineDotPass<uchar>>()->lineDots);
    t.timeEvent("Line Search");
  }

  // Find blobs
  auto blobsPerLabel = getHandler<BlobDetectPass>()->detectBlobs();
  t.timeEvent("Blob Detection");

  //
  // UPDATE STATE
  //

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

      if (larger.area < d_minBallArea->getValue())
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
    t.timeEvent("Ball Blob Merging");

    // The first is the biggest, topmost ball blob
    for (Blob const& ballBlob : ballBlobs)
    {
      if (ballBlob.area == 0)
      {
        // Ignore blobs that were previously merged into another blob (zero area)
        continue;
      }

      // Ignore balls that are too small (avoid noise)
      if (ballBlob.area < d_minBallArea->getValue())
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

      // Discard blobs that would be too large/small for the ball we expect at that position of the frame
      Vector2d pos;
      if (canBlobBeBall(ballBlob, &pos))
      {
        ballPosition = Maybe<Vector2d>(pos);
        break;
      }
    }
    t.timeEvent("Ball Blob Selection");
  }

  // Do we have goal posts?
  vector<Vector2d> goalPositions;
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

    if (wh.minCoeff() > d_minGoalDimensionPixels->getValue() &&  // Ignore small blobs
        wh.y() > wh.x())                                         // Taller than it is wide
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
  t.timeEvent("Goal Blob Selection");

  AgentState::getInstance().set(make_shared<CameraFrameState const>(ballPosition, goalPositions, observedLineSegments, totalPixelCount, processedPixelCount));

  t.timeEvent("Updating State");
}
