#include <iostream>
#include <time.h>
#include <sys/time.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "../geometry/Line.hh"
#include "../HoughLineAccumulator/houghlineaccumulator.hh"
#include "../HoughLineExtractor/houghlineextractor.hh"
#include "../ImageLabeller/imagelabeller.hh"
#include "../ImagePassRunner/imagepassrunner.hh"
#include "../ImagePassHandler/BlobDetectPass/blobdetectpass.hh"
#include "../ImagePassHandler/CartoonPass/cartoonpass.hh"
#include "../ImagePassHandler/LabelCountPass/labelcountpass.hh"
#include "../ImagePassHandler/LineDotPass/linedotpass.hh"
#include "../LineRunTracker/lineruntracker.hh"
#include "../LineFinder/MaskWalkLineFinder/maskwalklinefinder.hh"
#include "../LineFinder/RandomPairLineFinder/randompairlinefinder.hh"
#include "../LUTBuilder/lutbuilder.hh"
#include "../PixelFilterChain/pixelfilterchain.hh"
#include "../PixelLabel/pixellabel.hh"

using namespace cv;
using namespace std;
using namespace bold;
using namespace Eigen;

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

  int loopCount = 20;

  auto inputFileName = argv[1];

  // Load the BGR image
  cout << "Reading " << inputFileName << endl;
  cv::Mat colourImage = imread(inputFileName, CV_LOAD_IMAGE_COLOR);

  if (!colourImage.data)
  {
    cout << "Could not open or find the image" << endl;
    return -1;
  }

  // Convert YCbCr to BGR
  PixelFilterChain chain;
  chain.pushFilter(&Colour::yCbCrToBgrInPlace);
  chain.applyFilters(colourImage);

  // Initialise random seed
  std::srand(unsigned(std::time(0)));

  int imageWidth = colourImage.cols;
  int imageHeight = colourImage.rows;

  //
  // FIXED START UP INITIALISATION
  //

  auto t = getTimestamp();

  // Build colour ranges for segmentation

  // hatfield (old, white field)
  shared_ptr<PixelLabel> ballLabel = make_shared<PixelLabel>(PixelLabel(Colour::hsvRange::fromDoubles(354,   6, 0.74, 0.18, 0.71, 0.22), "Ball")); // red super ball
  shared_ptr<PixelLabel> goalLabel = make_shared<PixelLabel>(PixelLabel(Colour::hsvRange::fromDoubles( 54,  15, 0.75, 0.20, 0.74, 0.20), "Goal")); // yellow paper
  shared_ptr<PixelLabel> fieldLabel= make_shared<PixelLabel>(PixelLabel(Colour::hsvRange::fromDoubles(  0, 360, 0.00, 0.25, 0.85, 0.35), "Field")); // white floor
  shared_ptr<PixelLabel> lineLabel = make_shared<PixelLabel>(PixelLabel(Colour::hsvRange::fromDoubles(  0, 360, 0.00, 0.75, 0.00, 0.75), "Line")); // black line

  // rgb.jpg
//   shared_ptr<PixelLabel> ballLabel  = make_shared<PixelLabel>(PixelLabel(Colour::hsvRange(13, 30, 255, 95, 190, 95), "Ball"));
//   shared_ptr<PixelLabel> goalLabel  = make_shared<PixelLabel>(PixelLabel(Colour::hsvRange(40, 10, 210, 55, 190, 65), "Goal"));
//   shared_ptr<PixelLabel> fieldLabel = make_shared<PixelLabel>(PixelLabel(Colour::hsvRange(71, 20, 138, 55, 173, 65), "Field"));
//   shared_ptr<PixelLabel> lineLabel  = make_shared<PixelLabel>(PixelLabel(Colour::hsvRange(0, 255, 0, 70, 255, 70), "Line"));

  cout << "Using labels:" << endl
       << "  " << *ballLabel << endl
       << "  " << *goalLabel << endl
       << "  " << *fieldLabel << endl
       << "  " << *lineLabel << endl;

  vector<shared_ptr<PixelLabel>> labels = { goalLabel, ballLabel, fieldLabel, lineLabel };

  // Resources for labelling
  cv::Mat labelledImage(colourImage.size(), CV_8UC1);
  auto imageLabeller = new ImageLabeller(LUTBuilder::buildLookUpTableBGR18(labels));

  // Resources for blob detection
  auto ballUnionPred = &Run::overlaps;
  auto goalUnionPred =
    [] (Run const& a, Run const& b)
    {
      if (!a.overlaps(b))
        return false;

      float ratio = (float)a.length() / (float)b.length();
      return min(ratio, 1.0f / ratio) > 0.75;
    };

  const vector<BlobType> blobTypes = {
    BlobType(ballLabel, ballUnionPred),
    BlobType(goalLabel, goalUnionPred)
  };
  auto blobDetectPass = make_shared<BlobDetectPass>(imageWidth, imageHeight, blobTypes);

  // Resources for finding line dots
  auto lineDotPass = make_shared<LineDotPass<uchar>>(imageWidth, fieldLabel, lineLabel, 3);

  // Resources for creating a labelled image
  auto cartoonPass = make_shared<CartoonPass>(imageWidth, imageHeight, labels, Colour::bgr(128,128,128));

  // Resources for counting the number of labels
  auto labelCountPass = make_shared<LabelCountPass>(labels);

  // Build the image passer
  vector<shared_ptr<ImagePassHandler<uchar>>> handlers = {
    lineDotPass,
    blobDetectPass,
    cartoonPass,
    labelCountPass
  };
  auto passer = ImagePassRunner<uchar>(handlers);

  RandomPairLineFinder randomPairLineFinder(imageWidth, imageHeight);
  randomPairLineFinder.setMinDotManhattanDistance(10);
//randomPairLineFinder.setProcessDotCount(5000);

  MaskWalkLineFinder maskWalkLineFinder(imageWidth, imageHeight);

  cout << "Startup took " << (getSeconds(t)*1000) << " ms" << endl;

  //
  // IMAGE LABELLING
  //
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
  cout << "Passed " << loopCount << " times. Average time: " << (getSeconds(t)*1000/loopCount) << " ms" << endl;

  //
  // FIND LINES (RandomPairLineFinder)
  //
  t = getTimestamp();
  vector<LineSegment2i> randomPairLines;
  for (int i = 0; i < loopCount; i++)
    randomPairLines = randomPairLineFinder.findLineSegments(lineDotPass->lineDots);
  cout << "RandomPairLineFinder ran " << loopCount << " times. Average time: " << (getSeconds(t)*1000/loopCount) << " ms" << endl;

  //
  // FIND LINES (RandomPairLineFinder)
  //
  t = getTimestamp();
  vector<LineSegment2i> maskWalkLines;
  for (int i = 0; i < loopCount; i++)
    maskWalkLines = maskWalkLineFinder.findLineSegments(lineDotPass->lineDots);
  cout << "MaskWalkLineFinder   ran " << loopCount << " times. Average time: " << (getSeconds(t)*1000/loopCount) << " ms" << endl;

  //
  // PRINT SUMMARIES
  //
  cout << "Finished " << loopCount << " passes. Average time: " << (getSeconds(t)*1000/loopCount) << " ms" << endl
       << "Found:" << endl
       << "    " << lineDotPass->lineDots.size() << " line dots" << endl;

  auto blobsByLabel = blobDetectPass->getDetectedBlobs();
  for (BlobType const& blobType : blobTypes)
    cout << "    " << blobsByLabel[blobType.pixelLabel].size() << " " << blobType.pixelLabel->name() << " blob(s)" << endl;
  for (auto const& pair : labelCountPass->getCounts())
    cout << "    " << pair.second << " " << pair.first->name() << " pixels" << endl;

  //
  // DRAW LABELLED 'CARTOON' IMAGE
  //
  imwrite("labelled.png", cartoonPass->mat());

  //
  // DRAW OUTPUT IMAGE
  //

  vector<Colour::bgr> colours = {
    Colour::bgr(255,0,0), // blue
    Colour::bgr(0,255,0), // green
    Colour::bgr(0,0,255), // red
    Colour::bgr(0,255,255), // yellow
    Colour::bgr(255,0,255), // magenta
    Colour::bgr(255,255,0) // cyan
  };

  // Draw line dots
//   cv::Mat lineDotImageGray(colourImage.size(), CV_8U);
//   lineDotImageGray = Scalar(0);
  if (lineDotPass->lineDots.size() != 0)
  {
    cv::Mat lineDotImageColour = cv::Mat::zeros(colourImage.size(), CV_8UC3);
    Colour::bgr red(0, 0, 255);
    for (Vector2i const& lineDot : lineDotPass->lineDots)
    {
//       lineDotImageGray.at<uchar>(lineDot.y(), lineDot.x()) = 255;
      colourImage.at<Colour::bgr>(lineDot.y(), lineDot.x()) = red;
      lineDotImageColour.at<Colour::bgr>(lineDot.y(), lineDot.x()) = red;
    }
    imwrite("line-dots.png", lineDotImageColour);
//     imwrite("line-dots-gray.bmp", lineDotImageGray);
  }

  // Draw line segments
//   vector<LineSegment2i> segments = lineFinder.find(lineDotPass->lineDots);
//   cout << "    " << segments.size() << " line segments" << endl;
//   for (LineSegment2i const& segment : segments)
//   {
//     cout << segment << endl;
//     segment.draw(colourImage, Colour::bgr(0,255,255));
//   }

  // Draw lines
  int colourIndex = 0;
  cout << "    " << randomPairLines.size() << " line(s) via RandomPairLineFinder" << endl;
  for (LineSegment2i const& line : randomPairLines)
  {
    cout << "      " << line << endl;
//     line.draw(colourImage, colours[colourIndex++ % colours.size()], 2);
  }
  cout << "    " << maskWalkLines.size() << " line(s) via MaskWalkLineFinder" << endl;
  for (LineSegment2i const& line : maskWalkLines)
  {
    cout << "      " << line << endl;
    line.draw(colourImage, colours[colourIndex++ % colours.size()], 2);
  }

  // Draw blobs
  for (BlobType const& blobType : blobTypes)
  for (bold::Blob blob : blobsByLabel[blobType.pixelLabel])
  {
    auto blobColor = blobType.pixelLabel->hsvRange().toBgr()/*.invert()*/.toScalar();
    cv::rectangle(colourImage, blob.toRect(), blobColor);
  }

  // Save output image
  imwrite("output.png", colourImage);

//   // Try using OpenCV's line detection
//   vector<Vec4i> cvLines;
//   cv::HoughLinesP(lineDotImageGray, cvLines, 2, 0.5*CV_PI/180, 10, 30, 40);
//   cout << "    " << cvLines.size() << " line(s) via OpenCV" << endl;
//   colourIndex = 0;
//   for (auto const& line : cvLines)
//   {
//     cv::line(colourImage,
//              Point(line[0], line[1]),
//              Point(line[2], line[3]),
//              colours[colourIndex++ % colours.size()].toScalar());//, 1, CV_AA);
//   }
//   imwrite("cv-lines.png", colourImage);
//   waitKey();

  return 0;
}
