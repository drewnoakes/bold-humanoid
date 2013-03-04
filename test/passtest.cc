#include <iostream>
#include <time.h>
#include <sys/time.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "../Geometry/geometry.hh"
#include "../HoughLineAccumulator/houghlineaccumulator.hh"
#include "../HoughLineExtractor/houghlineextractor.hh"
#include "../ImageLabeller/imagelabeller.hh"
#include "../ImagePasser/imagepasser.hh"
#include "../ImagePassHandler/BlobDetectPass/blobdetectpass.hh"
#include "../ImagePassHandler/CartoonPass/cartoonpass.hh"
#include "../ImagePassHandler/LabelCountPass/labelcountpass.hh"
#include "../ImagePassHandler/LineDotPass/linedotpass.hh"
#include "../LineRunTracker/lineruntracker.hh"
#include "../LineFinder/linefinder.hh"
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

  auto inputFileName = argv[1];

  // Load the colour image
  cv::Mat colourImage = imread(inputFileName, CV_LOAD_IMAGE_COLOR);

  if (!colourImage.data)
  {
    cout << "Could not open or find the image" << std::endl;
    return -1;
  }

  int imageWidth = colourImage.cols;
  int imageHeight = colourImage.rows;

  cout << "Reading " << inputFileName << endl;

  //
  // FIXED SETUP
  //

  auto t = getTimestamp();

  // Build colour ranges for segmentation

//   // sample-images
//   PixelLabel ballLabel (Colour::hsvRange::fromDoubles(354,   6, 0.74, 0.18, 0.71, 0.22), "Ball"); // red super ball
//   PixelLabel goalLabel (Colour::hsvRange::fromDoubles( 54,  15, 0.75, 0.20, 0.74, 0.20), "Goal"); // yellow paper
//   PixelLabel fieldLabel(Colour::hsvRange::fromDoubles(  0, 360, 0.00, 0.25, 0.85, 0.35), "Field"); // white floor
//   PixelLabel lineLabel (Colour::hsvRange::fromDoubles(  0, 360, 0.00, 0.45, 0.00, 0.45), "Line"); // black line

  // rgb.jpg
  PixelLabel ballLabel(Colour::hsvRange(13, 30, 255, 95, 190, 95), "Ball");
  PixelLabel goalLabel(Colour::hsvRange(40, 10, 210, 55, 190, 65), "Goal");
  PixelLabel fieldLabel(Colour::hsvRange(71, 20, 138, 55, 173, 65), "Field");
  PixelLabel lineLabel(Colour::hsvRange(0, 255, 0, 70, 255, 70), "Line");

  cout << ballLabel << endl;
  cout << goalLabel << endl;
  cout << fieldLabel << endl;
  cout << lineLabel << endl;

  vector<PixelLabel> labels = { goalLabel, ballLabel, fieldLabel, lineLabel };

  // Label the image
  cv::Mat labelledImage(colourImage.size(), CV_8UC1);
  auto imageLabeller = new ImageLabeller(labels);

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

  auto lineDotPass = new LineDotPass<uchar>(imageWidth, fieldLabel, lineLabel, 3);
  vector<BlobType> blobTypes = {
    BlobType(ballLabel, ballUnionPred),
    BlobType(goalLabel, goalUnionPred)
  };
  auto blobDetectPass = new BlobDetectPass(imageWidth, imageHeight, blobTypes);
  auto cartoonPass = new CartoonPass(imageWidth, imageHeight, labels, Colour::bgr(128,128,128));
  auto labelCountPass = new LabelCountPass(labels);

  vector<ImagePassHandler<uchar>*> handlers = {
    lineDotPass,
    blobDetectPass,
    cartoonPass,
    labelCountPass
  };

  auto passer = ImagePasser<uchar>(handlers);

  LineFinder lineFinder(imageWidth, imageHeight);

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

  vector<Line> lines;
  for (int i = 0; i < loopCount; i++)
  {
    passer.pass(labelledImage);
    lines = lineFinder.find(lineDotPass->lineDots, 15, 2000);
  }

  //
  // PRINT SUMMARIES
  //

  cout << "Finished " << loopCount << " passes. Average time: " << (getSeconds(t)*1000/loopCount) << " ms" << endl
       << "Found:" << endl
       << "    " << lineDotPass->lineDots.size() << " line dots" << endl;

  for (BlobType const& blobType : blobTypes)
  {
    PixelLabel pixelLabel = blobType.pixelLabel;
    size_t blobCount = blobDetectPass->blobsPerLabel[pixelLabel].size();
    cout << "    " << blobCount << " " << pixelLabel.name() << " blob(s)" << endl;
  }

  for (auto const& pair : labelCountPass->getCounts())
  {
    cout << "    " << pair.second << " " << pair.first.name() << " pixels" << endl;
  }

  //
  // DRAW LABELLED 'CARTOON' IMAGE
  //
  imwrite("labelled.jpg", cartoonPass->mat());

  //
  // DRAW OUTPUT IMAGE
  //

  // Draw line dots
  if (lineDotPass->lineDots.size() != 0)
  {
    for (Eigen::Vector2i const& lineDot : lineDotPass->lineDots) {
      Colour::bgr lineColor(0, 0, 255); // red
      colourImage.at<Colour::bgr>(lineDot.y(), lineDot.x()) = lineColor;
    }
  }

//   vector<LineSegment2i> segments = lineFinder.find(lineDotPass->lineDots);
//   cout << "    " << segments.size() << " line segments" << endl;
//   for (LineSegment2i const& segment : segments)
//   {
//     cout << "      between (" << segment.p1().x() << "," << segment.p1().y() << ") and ("
//                               << segment.p2().x() << "," << segment.p2().y() << ")" << endl;
//     segment.draw(colourImage, Colour::bgr(0,255,255));
//   }

  cout << "    " << lines.size() << " lines" << endl;
  for (Line const& line : lines)
  {
    cout << "      theta=" << line.theta() << " (" << (line.thetaDegrees()) << " degs) radius=" << line.radius() << " m=" << line.gradient() << " c=" << line.yIntersection() << endl;
    line.draw(colourImage, Colour::bgr(0,255,255).toScalar());
  }

  // Draw blobs
  for (BlobType const& blobType : blobTypes)
  for (bold::Blob blob : blobDetectPass->blobsPerLabel[blobType.pixelLabel])
  {
    auto blobColor = blobType.pixelLabel.hsvRange().toBgr().invert().toScalar();
    cv::rectangle(colourImage, blob.toRect(), blobColor);
  }

  // Save output image
  imwrite("output.jpg", colourImage);

  return 0;
}