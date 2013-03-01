#include <iostream>
#include <time.h>
#include <sys/time.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "../HoughLine/houghline.hh"
#include "../HoughLineAccumulator/houghlineaccumulator.hh"
#include "../HoughLineExtractor/houghlineextractor.hh"
#include "../ImageLabeller/imagelabeller.hh"
#include "../ImagePasser/imagepasser.hh"
#include "../ImagePassHandler/BlobDetectPass/blobdetectpass.hh"
#include "../ImagePassHandler/LineDotPass/linedotpass.hh"
#include "../LineRunTracker/lineruntracker.hh"
#include "../LUTBuilder/lutbuilder.hh"
#include "../PixelLabel/pixellabel.hh"

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

  int imageWidth = colourImage.cols;
  int imageHeight = colourImage.rows;

  cout << "Reading " << argv[1] << endl;

  //
  // FIXED SETUP
  //

  auto t = getTimestamp();

  // Build colour ranges for segmentation
  PixelLabel ballLabel(Colour::hsvRange(13, 30, 255, 95, 190, 95), "Ball");
  PixelLabel goalLabel(Colour::hsvRange(40, 10, 210, 55, 190, 65), "Goal");
  PixelLabel fieldLabel(Colour::hsvRange(71, 20, 138, 55, 173, 65), "Field");
  PixelLabel lineLabel(Colour::hsvRange(0, 255, 0, 70, 255, 70), "Line");

  vector<PixelLabel> labels = { ballLabel, goalLabel, fieldLabel, lineLabel };

  // Label the image
  cv::Mat labelledImage(colourImage.size(), CV_8UC1);
  auto imageLabeller = new ImageLabeller(labels);

  // Draw out the labelled image
//  cv::Mat colourLabelledImage(colourImage.size(), CV_8UC3);
//  ImageLabeller::colourLabels(labelledImage, colourLabelledImage, labels);
//  imwrite("labelled.jpg", colourLabelledImage);

  auto ballUnionPred =
    [] (Run const& a, Run const& b)
    {
      return max(a.end.x(), b.end.x()) - min(a.start.x(), b.start.x()) <= a.length + b.length;
    };

  auto goalUnionPred =
    [] (Run const& a, Run const& b)
    {
      if (max(a.end.x(), b.end.x()) - min(a.start.x(), b.start.x()) > a.length + b.length)
        return false;

      float ratio = (float)a.length / (float)b.length;
      return min(ratio, 1.0f / ratio) > 0.75;
    };

  vector<UnionPredicate> unionPredicateByLabel = {goalUnionPred, ballUnionPred};

  auto lineDetect = new LineDotPass<uchar>(imageWidth, fieldLabel, lineLabel, 1);
  vector<BlobLabel> blobLabels = {
    BlobLabel(ballLabel, ballUnionPred),
    BlobLabel(goalLabel, goalUnionPred)
  };
  auto blobDetect = new BlobDetectPass(imageWidth, imageHeight, blobLabels);

  vector<ImagePassHandler<uchar>*> handlers = { lineDetect, blobDetect };

  auto passer = ImagePasser<uchar>(handlers);

  cout << "Startup took " << (getSeconds(t)*1000) << " ms" << endl;

  //
  // IMAGE LABELLING
  //

  int loopCount = 20;

  t = getTimestamp();

  for (int i = 0; i < loopCount; i++)
    imageLabeller->label(colourImage, labelledImage);

  cout << "Labelled " << loopCount << " times. Average time: " << (getSeconds(t)*1000/loopCount) << " ms" << endl;

  //
  // IMAGE PASS
  //

  t = getTimestamp();

  for (int i = 0; i < loopCount; i++)
    passer.pass(labelledImage);

  //
  // PRINT SUMMARIES
  //

  cout << "Finished " << loopCount << " passes. Average time: " << (getSeconds(t)*1000/loopCount) << " ms" << endl
       << "Found:" << endl
       << "    " << lineDetect->lineDots.size() << " line dots" << endl;

  for (BlobLabel const& blobLabel : blobLabels)
  {
    PixelLabel pixelLabel = blobLabel.pixelLabel;
    size_t blobCount = blobDetect->blobsPerLabel[pixelLabel].size();
    cout << "    " << pixelLabel.name() << " " << blobCount << " blob(s)" << endl;
  }

  //
  // DRAW OUTPUT IMAGE
  //

  // Draw line dots
  if (lineDetect->lineDots.size() != 0)
  {
    for (Eigen::Vector2i const& lineDot : lineDetect->lineDots) {
      Colour::bgr lineColor(0, 0, 255); // red
      colourImage.at<Colour::bgr>(lineDot.y(), lineDot.x()) = lineColor;
    }
  }

  // Draw lines
//     double maxVotes = lineDetect->lines[0].votes();
//     for (bold::HoughLine const& line : lineDetect->lines) {
// //    cout << "  theta=" << line.theta() << " (" << (line.theta()*180.0/M_PI) << " degs) radius=" << line.radius() << " m=" << line.gradient() << " c=" << line.yIntersection() << endl;
//       Colour::bgr lineColor(0, 0, 255 * (line.votes()/maxVotes)); // red
//       line.draw<Colour::bgr>(colourImage, lineColor);
//     }

  // Draw blobs
  for (BlobLabel const& blobLabel : blobLabels)
  for (bold::Blob blob : blobDetect->blobsPerLabel[blobLabel.pixelLabel])
  {
    auto size = blob.br - blob.ul;
    cv::Rect rect(blob.ul.x(), blob.ul.y(), size.x(), size.y());
    // TODO create and use function to get label's colour
    Colour::bgr blobColor(255, 0, 0);
    cv::rectangle(colourImage, rect, cv::Scalar(blobColor.b, blobColor.g, blobColor.r));
  }

  // Save output image
  imwrite("output.jpg", colourImage);

  return 0;
}