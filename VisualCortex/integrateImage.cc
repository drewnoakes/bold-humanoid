#include "visualcortex.ih"

#include "../DataStreamer/datastreamer.hh"
#include "../vision/PixelFilterChain/pixelfilterchain.hh"

void VisualCortex::integrateImage(cv::Mat& image, DataStreamer* streamer)
{
  auto& debugger = Debugger::getInstance();
  auto t = Debugger::getTimestamp();

  //
  // PROCESS THE IMAGE
  //

  // Label the image;
  // OPT: make data member
  static cv::Mat labelled(image.rows, image.cols, CV_8UC1);
  d_imageLabeller->label(image, labelled);
  t = debugger.timeEvent(t, "Image Processing/Pixel Label");

  // Perform the image pass
  d_imagePasser->pass(labelled);
  t = debugger.timeEvent(t, "Image Processing/Pass");

  // Find lines
  d_lines = d_lineFinder->find(d_lineDotPass->lineDots);
  t = debugger.timeEvent(t, "Image Processing/Line Search");

  //
  // UPDATE STATE
  //

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

  t = debugger.timeEvent(t, "Image Processing/Updating State");


  //
  // DEBUG IMAGES
  //

  if (streamer->shouldProvideImage())
  {
    ImageType imageType = streamer->getImageType();

    Mat debugImage;

    if (imageType == ImageType::YCbCr)
    {
      debugImage = image;
    }
    else if (imageType == ImageType::RGB)
    {
      debugImage = image;
      PixelFilterChain chain;
      chain.pushFilter([](unsigned char* pxl) {
        Colour::YCbCr* ycbcr = reinterpret_cast<Colour::YCbCr*>(pxl);
        Colour::bgr* bgr = reinterpret_cast<Colour::bgr*>(pxl);
        *bgr = (*ycbcr).toBgrInt();
      });
      chain.applyFilters(debugImage);
    }
    else if (imageType == ImageType::Cartoon)
    {
      debugImage = d_cartoonPass->mat();
    }
    else if (imageType == ImageType::None)
    {
      debugImage = Mat(image.size(), image.type(), Scalar(0));
    }

    if (streamer->drawLines() && d_lines.size() > 0)
    {
      // TODO pull this selection code out
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
        cv::line(debugImage,
                cv::Point(hypothesis.min().x(), hypothesis.min().y()),
                cv::Point(hypothesis.max().x(), hypothesis.max().y()),
                Colour::bgr(255,0,0).toScalar(),
                2);
      }
    }

    streamer->streamImage(debugImage);
    t = debugger.timeEvent(t, "Image Processing/Image Streaming");
  }
}
