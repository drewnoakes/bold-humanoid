#include "visualcortex.ih"

void VisualCortex::integrateImage(Mat& image, SequentialTimer& t)
{
  //
  // Record frame, if required
  //
  static Clock::Timestamp lastRecordTime;
  if (d_saveNextYUVFrame || (d_isRecordingYUVFrames->getValue() && Clock::getSecondsSince(lastRecordTime) > 1.0))
  {
    saveImage(image);
    lastRecordTime = Clock::getTimestamp();
    d_saveNextYUVFrame = false;
    t.timeEvent("Save YUV Frame");
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
//long processedPixelCount = d_imagePassRunner->pass(d_labelledImage, d_granularityFunction, t);
  long processedPixelCount = d_imagePassRunner->passWithHandlers(d_imagePassHandlers, d_labelledImage, d_granularityFunction, t);

  long totalPixelCount = d_labelledImage.rows * d_labelledImage.cols;

  if (d_shouldCountLabels->getValue())
  {
    State::set(make_shared<LabelCountState const>(getHandler<LabelCountPass>()->getCounts()));
    t.timeEvent("Store Label Count");
  }

  // Find lines
  vector<LineSegment2i> observedLineSegments;
  if (d_shouldDetectLines->getValue())
  {
    observedLineSegments = d_lineFinder->findLineSegments(getHandler<LineDotPass<uchar>>()->lineDots);
    t.timeEvent("Line Search");
  }

  Maybe<Vector2d> ballPosition = Maybe<Vector2d>::empty();
  vector<Vector2d,aligned_allocator<Vector2d>> goalPositions;

  if (d_shouldDetectBlobs->getValue())
  {
    // Find blobs
    t.enter("Blob Detect");
    auto blobsPerLabel = getHandler<BlobDetectPass>()->detectBlobs(t);
    t.exit();

    //
    // UPDATE STATE
    //

    // Might we have a ball?
    if (blobsPerLabel[d_ballLabel].size() > 0)
    {
      vector<Blob>& ballBlobs = blobsPerLabel[d_ballLabel];

      if (d_ballBlobMergingEnabled->getValue())
      {
        // Merge ball blobs
        for (int i = 0; i < min(10, (int)ballBlobs.size()); ++i)
        {
          Blob& larger = ballBlobs[i];

          if (larger.area == 0)
            continue;

          if (larger.area < unsigned(d_minBallAreaPixels->getValue()))
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
      }

      // The first is the biggest, topmost ball blob
      for (Blob const& ballBlob : ballBlobs)
      {
        // Ignore balls that are too small (avoid noise)
        if (ballBlob.area < unsigned(d_minBallAreaPixels->getValue()))
        {
          // As blobs are sorted largest to smallest, stop at the first one that's too small
          break;
        }

        // Filter out invalid ball blobs
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
    vector<Blob,aligned_allocator<Blob>> acceptedGoalBlobs;
    int allowedGoalFieldEdgeDistPixels = d_maxGoalFieldEdgeDistPixels->getValue();
    int minGoalDimensionPixels = d_minGoalDimensionPixels->getValue();
    for (Blob const& goalBlob : blobsPerLabel[d_goalLabel])
    {
      // Ignore goal if it appears outside of field
      //
      // NOTE Process this before anything else as anything above the field edge is wasting our time
      if (goalBlob.ul.y() > int(d_fieldEdgePass->getEdgeYValue(goalBlob.mean.x())) + allowedGoalFieldEdgeDistPixels)
        continue;

      // TODO apply this filtering earlier, so that the debug image doesn't show unused goal blobs
      Vector2i wh = goalBlob.br - goalBlob.ul;

      if (wh.minCoeff() > minGoalDimensionPixels && // Ignore small blobs
          wh.y() > wh.x())                          // Taller than it is wide
      {
        // Verify this blob does not overlap with a goal blob which was already accepted
        for (auto const& other : acceptedGoalBlobs)
        {
          // Blobs are sorted by size, descending.
          // If a smaller goal blob intersects a larger blob that we already
          // accepted as a goal, ignore the smaller one.
          if (goalBlob.bounds().overlaps(other.bounds()))
            continue;
        }

        // Discard blobs that would be too wide/narrow for the goal we expect at that position of the frame
        Vector2d pos;
        if (!canBlobBeGoal(goalBlob, &pos))
          continue;

        goalPositions.push_back(pos);
        acceptedGoalBlobs.push_back(goalBlob);
      }
    }
    t.timeEvent("Goal Blob Selection");

    if (log::minLevel <= LogLevel::Verbose && acceptedGoalBlobs.size() > 2)
    {
      // It's pretty rare that we should see three goal posts, so log information about the blobs
      log::verbose("VisualCortex::integrateImage") << acceptedGoalBlobs.size() << " accepted goal blobs";
      for (Blob const& goalBlob : acceptedGoalBlobs)
      {
        log::verbose("VisualCortex::integrateImage")
          << goalBlob.br.x() << ","
          << goalBlob.br.y() << ","
          << goalBlob.ul.x() << ","
          << goalBlob.ul.y();
      }
    }
  }

  State::set(make_shared<CameraFrameState const>(ballPosition, goalPositions, observedLineSegments, d_fieldEdgePass->getOcclusionRays(), totalPixelCount, processedPixelCount));

  t.timeEvent("Updating State");
}
