#include <iostream>
#include <time.h>
#include <sys/time.h>
//#include <stdio.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "../ImageLabeller/imagelabeller.hh"
#include "../LUTBuilder/lutbuilder.hh"

#include "../LineRunTracker/lineruntracker.hh"
#include "../HoughLine/houghline.hh"
#include "../HoughLineAccumulator/houghlineaccumulator.hh"
#include "../HoughLineExtractor/houghlineextractor.hh"
#include "../ImagePasser/imagepasser.hh"
#include "../ImagePassHandler/LineDetectPass/linedetectpass.hh"
#include "../ImagePassHandler/BlobDetectPass/blobdetectpass.hh"

using namespace cv;
using namespace std;
using namespace bold;

typedef unsigned long long timestamp_t;

const timestamp_t getTimestamp()
{
  struct timeval now;
  gettimeofday(&now, NULL);
  return now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
}

const double getSeconds(timestamp_t const& startedAt)
{
  auto now = getTimestamp();
  return (now - startedAt) / 1000000.0L;
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    cout <<" Usage: passtest <rgb-image>" << endl;
    return -1;
  }

  // Load the colour image
  cv::Mat colourImage = imread(argv[1], CV_LOAD_IMAGE_COLOR);

  if (!colourImage.data)
  {
    cout << "Could not open or find the image" << std::endl;
    return -1;
  }

  cout << "Reading " << argv[1] << endl;

  // Build colour ranges for segmentation
  Colour::hsvRange ballRange  = Colour::hsvRange(13, 30, 255, 95, 190, 95);
  Colour::hsvRange goalRange  = Colour::hsvRange(40, 10, 210, 55, 190, 65);
  Colour::hsvRange fieldRange = Colour::hsvRange(71, 20, 138, 55, 173, 65);
  Colour::hsvRange lineRange  = Colour::hsvRange(0, 255, 0, 70, 255, 70);

  vector<Colour::hsvRange> ranges;
  ranges.push_back(ballRange);
  ranges.push_back(goalRange);
  ranges.push_back(fieldRange);
  ranges.push_back(lineRange);

  //////////////// START TIMING

  // Label the image
  cv::Mat labelledImage(colourImage.size(), CV_8UC1);
  auto imageLabeller = new ImageLabeller(ranges);
  imageLabeller->label(colourImage, labelledImage);

  // Draw out the labelled image
  // TODO why does commenting these three lines out stop the line detector working???
  cv::Mat colourLabelledImage(colourImage.size(), CV_8UC3);
  ImageLabeller::colourLabels(labelledImage, colourLabelledImage, ranges);
  imwrite("labelled.jpg", colourLabelledImage);

  auto t = getTimestamp();

  auto ballUnionPred =
    [] (Run const& a, Run const& b)
    {
//    float ratio = (float)a.length / (float)b.length;
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

  vector<UnionPredicate> unionPredicateByLabel = {goalUnionPred, ballUnionPred};

  // TODO how to specify which labels are blobbed?

  auto lineDetect = new LineDetectPass<uchar>(labelledImage.cols, labelledImage.rows, 50, 720, 3, 4, 1);
  auto blobDetect = new BlobDetectPass(labelledImage.cols, labelledImage.rows, 2, unionPredicateByLabel);

  vector<ImagePassHandler<uchar>*> handlers = { lineDetect, blobDetect };

  auto passer = ImagePasser<uchar>(handlers);

  // TODO provide the colour image, and have the passer use the LUT
  passer.pass(labelledImage);

  cout << "Finished pass in " << getSeconds(t) << " seconds" << endl;

  //////////////// STOP TIMING

  //
  // Write the accumulator image out to a file
  //
  cv::Mat accImg = lineDetect->accumulator.getMat().clone();
//cv::rectangle(accImg, Point(0,0), Point(accImg.cols-1, accImg.rows-1), Scalar(1));
  cv::normalize(accImg, accImg, 0, 255, NORM_MINMAX, CV_16UC1);
  imwrite("accumulator.jpg", accImg);

  auto foundLines = lineDetect->lines.size() != 0;
  auto foundBlobs = blobDetect->blobsPerLabel.size() != 0; // TODO this is incorrect

  if (!foundLines && !foundBlobs)
  {
    cout << "Found nothing. Exiting." << endl;
    return 1;
  }

  cout << "Found " << lineDetect->lines.size() << " line(s), and" << endl;
  for (uchar label = 0; label < blobDetect->blobsPerLabel.size(); label++)
  {
    cout << "    " << blobDetect->blobsPerLabel[label].size() << " blob(s) for label " << (label + 1) << endl;
  }

  //
  // Draw lines
  //
  if (foundLines)
  {
    double maxVotes = lineDetect->lines[0].votes();
    for (bold::HoughLine const& line : lineDetect->lines) {
      cout << "  theta=" << line.theta() << " (" << (line.theta()*180.0/M_PI) << " degs) radius=" << line.radius() << " m=" << line.gradient() << " c=" << line.yIntersection() << endl;
      Colour::bgr lineColor(0, 0, 255 * (line.votes()/maxVotes)); // red
      line.draw<Colour::bgr>(colourImage, lineColor);
    }
  }

  //
  // Draw blobs
  //
  if (foundBlobs)
  {
    uchar numLabels = blobDetect->blobsPerLabel.size(); // TODO revisit this
    for (uchar label = 1; label < numLabels; label++)
    for (bold::Blob blob : blobDetect->blobsPerLabel[label])
    {
      auto size = blob.br - blob.ul;
      cv::Rect rect(blob.ul.x(), blob.ul.y(), size.x(), size.y());
      // TODO create and use function to get label's colour
      Colour::bgr blobColor(255, 0, 0);
      cv::rectangle(colourImage, rect, Scalar(blobColor.b, blobColor.g, blobColor.r));
    }
  }

  imwrite("output.jpg", colourImage);

  return 0;
}