#include "visualcortex.hh"

#include <cmath>

#include "../Debugger/debugger.hh"

using namespace std;
using namespace bold;
using namespace Eigen;

void VisualCortex::integrateImage(cv::Mat& image, DataStreamer* streamer)
{
//  cout << "[VisualCortex::integrateImage] Start" << endl;
//  cout << "[VisualCortex::integrateImage] Image size: " << image.rows << "x" << image.cols << endl;

  auto t = Debugger::getTimestamp();

  //
  // Send the image via debugger (if required)
  //
  if (streamer)
    streamer->streamImage(image, "raw");
//   t = debugger.timeEvent(t, "Image Streaming");


  // TODO label the iamge directly from YUV (if we don't already)
  // convert from YUV to RGB
  d_pfChain.applyFilters(image);

  // TODO time all events in here
//  t = debugger.timeEvent(t, "Pixel Filter Chain");

  // Label the image;
  // OPT: make data memeber
  static cv::Mat labelled(image.rows, image.cols, CV_8UC1);
  // TODO ensure labeller sets all pixel values -- not just non-zero
  d_imageLabeller->label(image, labelled);


  if (streamer)
    streamer->streamImage(labelled, "labelled");

  d_imagePasser->pass(labelled);

  d_lines = d_lineFinder->find(d_lineDotPass->lineDots);

  auto blobsPerLabel = d_blobDetectPass->blobsPerLabel;

//  cout << "[VisualCortex::integrateImage] Blobs 0: " << blobs[0].size() << endl;
//  cout << "[VisualCortex::integrateImage] Blobs 1: " << blobs[1].size() << endl;

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
      postObs.pos = (topRun.end + topRun.start).cast<float>() / 2;

      d_observations.push_back(postObs);
      d_goalObservations.push_back(postObs);
    }
  }

//   Debugger::getInstance().processBlobs(blobs);

  /*
  if (d_showUI)
  {
    // Draw rectangles on the colour image
    for (set<Blob>& blobSet : blobs)
    {
      for (Blob const& b : blobSet)
      {
        if ((b.br - b.ul).minCoeff() > 5)
          cv::rectangle(image,
                        cv::Rect(b.ul.x(), b.ul.y(),
                                 b.br.x() - b.ul.x(), b.br.y() - b.ul.y()),
                        cv::Scalar(255,0,0),
                        2);
      }
    }

    for (Observation const& obs : observations)
    {
      cv::Scalar color;
      switch (obs.type)
      {
        case O_BALL:      color = cv::Scalar(0,0,255);   break;
        case O_GOAL_POST: color = cv::Scalar(0,255,255); break;
      }
      cv::circle(image, cv::Point(obs.pos.x(), obs.pos.y()), 5, color, 2);
    }

    cv::imshow("raw", image);
    cv::normalize(labelled, labelled, 0, 255, CV_MINMAX);
    cv::imshow("labelled", labelled);

    cv::waitKey(1);
  }
  */
}
