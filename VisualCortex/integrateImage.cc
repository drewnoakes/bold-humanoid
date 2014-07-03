#include "visualcortex.ih"

void VisualCortex::integrateImage(Mat& image, SequentialTimer& t, ulong thinkCycleNumber)
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

  if (d_labelTeacher->requestedSnapShot())
  {
    log::info("Label Teacher") << "Setting train image";
    d_labelTeacher->setTrainImage(image); // TODO: convert to HSV here, or in teacher?
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
    State::make<LabelCountState>(getHandler<LabelCountPass>()->getCounts());
    t.timeEvent("Store Label Count");
  }

  // Find lines
  vector<LineSegment2i> observedLineSegments;
  if (d_shouldDetectLines->getValue())
  {
    t.enter("Line Search");
    observedLineSegments = d_lineFinder->findLineSegments(getHandler<LineDotPass<uchar>>()->lineDots);
    t.exit();
  }

  Maybe<Vector2d> ballPosition = Maybe<Vector2d>::empty();
  vector<Vector2d,aligned_allocator<Vector2d>> goalPositions;
  vector<Vector2d,aligned_allocator<Vector2d>> teamMatePositions;

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
      auto& ballBlobs = blobsPerLabel[d_ballLabel];
      ballPosition = detectBall(ballBlobs, t);
    }

    // Do we have goal posts?
    if (blobsPerLabel[d_goalLabel].size() > 0)
    {
      auto& goalBlobs = blobsPerLabel[d_goalLabel];
      goalPositions = detectGoal(goalBlobs, t);
    }

    auto teamColour = Config::getStaticValue<TeamColour>("team-colour");

    auto ourColourLabel = teamColour == TeamColour::Cyan ? d_cyanLabel : d_magentaLabel;

    // Do we have own teammate blobs?
    if (d_playerDetectionEnabled->getValue() && blobsPerLabel[ourColourLabel].size() > 0)
    {
      auto& playerBlobs = blobsPerLabel[ourColourLabel];
      teamMatePositions = detectPlayers(playerBlobs, t);
    }
  }

  vector<OcclusionRay<ushort>> occlusionRays = d_fieldEdgePass->getOcclusionRays();

  // TODO do this widening in the image frame, not during projection
  static auto widenOcclusions = Config::getSetting<bool>("vision.occlusion.widen");
  static auto midHeightToWiden = Config::getSetting<int>("vision.occlusion.widen-min-height-px");

  if (widenOcclusions->getValue())
  {
    vector<OcclusionRay<ushort>> widenedRays;
    for (uint i = 0; i < occlusionRays.size(); i++)
    {
      auto const& ray = occlusionRays[i];
      Matrix<ushort,2,1> near = ray.near();
      if ((ray.far().y() - ray.near().y()) > midHeightToWiden->getValue())
      {
        // Widen occlusions by taking the near as the lowest of itself and its neighbours in the image
        if (i != 0)
          near.y() = min(near.y(), occlusionRays[i - 1].near().y());
        if (i != occlusionRays.size() - 1)
          near.y() = min(near.y(), occlusionRays[i + 1].near().y());
      }
      widenedRays.emplace_back(near, ray.far());
    }

    occlusionRays = widenedRays;
  }

  State::make<CameraFrameState>(ballPosition, goalPositions, teamMatePositions,
                                observedLineSegments, occlusionRays,
                                totalPixelCount, processedPixelCount, thinkCycleNumber);

  t.timeEvent("Updating State");
}
