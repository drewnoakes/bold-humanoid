#include "worldmodel.hh"

#include <cmath>
#include "../DataStreamer/datastreamer.hh" 
using namespace std;
using namespace bold;
using namespace Eigen;

void WorldModel::integrateImage(cv::Mat& image, DataStreamer* streamer)
{
//  cout << "[Agent::processImage] Start" << endl;
//  cout << "[Agent::processImage] Image size: " << image.rows << "x" << image.cols << endl;

  // Label the image;
  // OPT: make data memeber
  cv::Mat labelled(image.rows, image.cols, CV_8UC1);
  if (streamer)
    streamer->streamImage(labelled, "labelled");

  d_imageLabeller->label(image, labelled);

  auto ballUnionPred =
    [] (Run const& a, Run const& b)
    {
      float ratio = (float)a.length / (float)b.length;
      return
      max(a.end.x(), b.end.x()) - min(a.start.x(), b.start.x()) <= a.length + b.length;
    };


  auto goalUnionPred =
    [] (Run const& a, Run const& b)
    {
      float ratio = (float)a.length / (float)b.length;
      return
      max(a.end.x(), b.end.x()) - min(a.start.x(), b.start.x()) <= a.length + b.length &&
      min(ratio, 1.0f/ratio) > 0.75;
    };

  vector<function<bool(Run const&,Run const&)> > unionPreds = {goalUnionPred, ballUnionPred};

  BlobDetector detector;
  vector<set<Blob> > blobByLabel = detector.detectBlobs(labelled, 2, unionPreds);

//  cout << "[Agent::processImage] Blobs 0: " << blobs[0].size() << endl;
//  cout << "[Agent::processImage] Blobs 1: " << blobs[1].size() << endl;

  // Do we have a ball?
  isBallVisible = false;
  if (blobByLabel[1].size() > 0)
  {
    // The first is the biggest, topmost ball blob
    Blob const& ball = *blobByLabel[1].begin();

    if (ball.area > d_minBallArea)
    {
      Observation ballObs;
      ballObs.type = O_BALL;
      ballObs.pos = ball.mean;

      observations.push_back(ballObs);
      ballObservation = ballObs;
      isBallVisible = true;
    }
  }

  // Do we have goal posts?
  for (Blob const& b : blobByLabel[0])
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

      observations.push_back(postObs);
      goalObservations.push_back(postObs);
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
