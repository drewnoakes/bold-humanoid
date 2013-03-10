#include "visualcortex.hh"

#include <cmath>

#include "../Debugger/debugger.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

void VisualCortex::integrateImage(cv::Mat& image, DataStreamer* streamer)
{
  static unsigned long long frameIndex = 0;
  bool transmitThisFrame = frameIndex++%d_streamFramePeriod == 0;

  auto& debugger = Debugger::getInstance();

  auto t = Debugger::getTimestamp();

  if (transmitThisFrame && streamer)
  {
    streamer->streamImage(image, "raw");
    t = debugger.timeEvent(t, "Image Processing/Raw Image Streaming");
  }

  // TODO label the image directly from YUV (if we don't already)
  // convert from YUV to RGB
  d_pfChain.applyFilters(image);
  t = debugger.timeEvent(t, "Image Processing/Pixel Filters");

  // Label the image;
  // OPT: make data memeber
  static cv::Mat labelled(image.rows, image.cols, CV_8UC1);
  d_imageLabeller->label(image, labelled);
  t = debugger.timeEvent(t, "Image Processing/Pixel Label");

  d_imagePasser->pass(labelled);
  t = debugger.timeEvent(t, "Image Processing/Pass");

  d_lines = d_lineFinder->find(d_lineDotPass->lineDots);
  t = debugger.timeEvent(t, "Image Processing/Line Search");

  if (transmitThisFrame && streamer)
  {
    cv::Mat cartoon = d_cartoonPass->mat();

    // TODO allow drawing debug info on any view, not just the cartoon one
    if (d_lines.size() > 0)
    {
      // Calculate the average vote count for the top N hypotheses
      int sumVotes = 0;
      int takeTop = 0;
      for (auto const& hypothesis : d_lines)
      {
        if (++takeTop == 15) // <-- controllable number
          break;
        sumVotes += hypothesis.count();
      }
      int averageVotes = sumVotes / takeTop;

      for (auto const& hypothesis : d_lines)
      {
        // Only take those with an above average number of votes
        if (hypothesis.count() < averageVotes)
          break;

        auto line = hypothesis.toLine();
        cv::line(cartoon,
                 cv::Point(hypothesis.min().x(), hypothesis.min().y()),
                 cv::Point(hypothesis.max().x(), hypothesis.max().y()),
                 Colour::bgr(255,0,0).toScalar(),
                 2);
      }

    }
    streamer->streamImage(cartoon, "labelled");
    t = debugger.timeEvent(t, "Image Processing/Labelled Image Stream");
  }

  auto blobsPerLabel = d_blobDetectPass->blobsPerLabel;
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

  t = debugger.timeEvent(t, "Image Processing/Finishing Up");
}
