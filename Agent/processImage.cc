#include "agent.ih"

vector<Observation> Agent::processImage(cv::Mat& image)
{
//  cout << "[Agent::processImage] Start" << endl;
//  cout << "[Agent::processImage] Image size: " << image.rows << "x" << image.cols << endl;

  // Label the image;
  // OPT: make data memeber
  cv::Mat labeled(image.rows, image.cols, CV_8UC1);

  for (unsigned y = 0; y < image.rows; ++y)
  {
    unsigned char *origpix = image.ptr<unsigned char>(y);
    unsigned char *labeledpix = labeled.ptr<unsigned char>(y);
    for (unsigned x = 0; x < image.cols; ++x)
    {
      unsigned char l = d_LUT[(origpix[0] << 16) | (origpix[1] << 8) | origpix[2]];
      *labeledpix = l;

      ++origpix;
      ++origpix;
      ++origpix;
      ++labeledpix;
    }
  }

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
  vector<set<Blob> > blobs = detector.detectBlobs(labeled, 2, unionPreds);

//  cout << "[Agent::processImage] Blobs 0: " << blobs[0].size() << endl;
//  cout << "[Agent::processImage] Blobs 1: " << blobs[1].size() << endl;

  vector<Observation> observations;

  // Do we have a ball?
  if (blobs[1].size() > 0)
  {
    Observation ballObs;
    ballObs.type = O_BALL;

    // Get the biggest top most ball blob. This is the first
    Blob const& ball = *blobs[1].begin();

    ballObs.pos = ball.mean;

    observations.push_back(ballObs);
  }

  // Do we have goal posts?
  for (Blob const& b : blobs[0])
  {
    Vector2i wh = b.br - b.ul;
    if (wh.minCoeff() > 5  &&  // ignore small blobs
        wh.y() > wh.x())       // Higher than it is lower
    {
      // Take center of top most run. This is the first
      Observation postObs;
      postObs.type = O_GOAL_POST;
      Run const& topRun = *b.runs.begin();
      postObs.pos = (topRun.end + topRun.start).cast<float>() / 2;

      observations.push_back(postObs);
    }
  }

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

  if (d_showUI)
  {
    cv::normalize(labeled, labeled, 0, 255, CV_MINMAX );
    cv::imshow("labeled", labeled);
  }

  return observations;
}
