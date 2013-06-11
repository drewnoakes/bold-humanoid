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
#include "../Clock/clock.hh"

using namespace cv;
using namespace std;
using namespace bold;
using namespace Eigen;

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
  Mat colourImage = imread(inputFileName, CV_LOAD_IMAGE_COLOR);

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

  auto t = Clock::getTimestamp();

  // Build colour ranges for segmentation

  // hatfield (old, white field)
//   shared_ptr<PixelLabel> ballLabel = make_shared<PixelLabel>(Colour::hsvRange::fromDoubles(354,   1, 0.74, 0.18, 0.71, 0.22), "Ball"); // red super ball
//   shared_ptr<PixelLabel> goalLabel = make_shared<PixelLabel>(Colour::hsvRange::fromDoubles( 54,  15, 0.75, 0.20, 0.74, 0.20), "Goal"); // yellow paper
//   shared_ptr<PixelLabel> fieldLabel= make_shared<PixelLabel>(Colour::hsvRange::fromDoubles(  0, 360, 0.00, 0.25, 0.85, 0.35), "Field"); // white floor
//   shared_ptr<PixelLabel> lineLabel = make_shared<PixelLabel>(Colour::hsvRange::fromDoubles(  0, 360, 0.00, 0.75, 0.00, 0.75), "Line"); // black line

  // rgb.jpg
  shared_ptr<PixelLabel> ballLabel  = make_shared<PixelLabel>(Colour::hsvRange(13, 30, 255, 95, 190, 95), "Ball");
  shared_ptr<PixelLabel> goalLabel  = make_shared<PixelLabel>(Colour::hsvRange(40, 10, 210, 55, 190, 65), "Goal");
  shared_ptr<PixelLabel> fieldLabel = make_shared<PixelLabel>(Colour::hsvRange(71, 20, 138, 55, 173, 65), "Field");
  shared_ptr<PixelLabel> lineLabel  = make_shared<PixelLabel>(Colour::hsvRange(0, 255, 0, 70, 255, 70), "Line");

  cout << "Using labels:" << endl
       << "  " << *ballLabel << endl
       << "  " << *goalLabel << endl
       << "  " << *fieldLabel << endl
       << "  " << *lineLabel << endl;

  vector<shared_ptr<PixelLabel>> labels = { goalLabel, ballLabel, fieldLabel, lineLabel };

  // Resources for labelling
  Mat labelledImage(colourImage.size(), CV_8UC1);
  // TODO: this will crash
  auto imageLabeller = new ImageLabeller(LUTBuilder::buildLookUpTableBGR18(labels), 0);

  const vector<shared_ptr<PixelLabel>> blobPixelLabels = { ballLabel, goalLabel };
  auto blobDetectPass = make_shared<BlobDetectPass>(imageWidth, imageHeight, blobPixelLabels);

  // Resources for finding line dots
  auto lineDotPass = make_shared<LineDotPass<uchar>>(imageWidth, fieldLabel, lineLabel, 3);

  // Resources for creating a labelled image
  auto cartoonPass = make_shared<CartoonPass>(imageWidth, imageHeight, labels, Colour::bgr(128,128,128));

  // Resources for counting the number of labels
  auto labelCountPass = make_shared<LabelCountPass>(labels);

  // Build the pass runner
  auto passRunner = ImagePassRunner<uchar>();
  passRunner.addHandler(lineDotPass);
  passRunner.addHandler(blobDetectPass);
  passRunner.addHandler(cartoonPass);
  passRunner.addHandler(labelCountPass);

  RandomPairLineFinder randomPairLineFinder(imageWidth, imageHeight);
  randomPairLineFinder.setMinDotManhattanDistance(10);
//randomPairLineFinder.setProcessDotCount(5000);

  MaskWalkLineFinder maskWalkLineFinder(imageWidth, imageHeight);

  cout << "Startup took " << Clock::getMillisSince(t) << " ms" << endl;

  //
  // IMAGE LABELLING
  //
  t = Clock::getTimestamp();
  for (int i = 0; i < loopCount; i++)
    imageLabeller->label(colourImage, labelledImage);
  cout << "Labelled " << loopCount << " times. Average time: " << (Clock::getMillisSince(t)/loopCount) << " ms" << endl;

  //
  // IMAGE PASS
  //
  t = Clock::getTimestamp();
  for (int i = 0; i < loopCount; i++)
    passRunner.pass(labelledImage);
  cout << "Passed " << loopCount << " times. Average time: " << (Clock::getMillisSince(t)/loopCount) << " ms" << endl;

  //
  // FIND LINES (RandomPairLineFinder)
  //
  t = Clock::getTimestamp();
  vector<LineSegment2i> randomPairLines;
  for (int i = 0; i < loopCount; i++)
    randomPairLines = randomPairLineFinder.findLineSegments(lineDotPass->lineDots);
  cout << "RandomPairLineFinder ran " << loopCount << " times. Average time: " << (Clock::getMillisSince(t)/loopCount) << " ms" << endl;

  //
  // FIND LINES (RandomPairLineFinder)
  //
  t = Clock::getTimestamp();
  vector<LineSegment2i> maskWalkLines;
  for (int i = 0; i < loopCount; i++)
    maskWalkLines = maskWalkLineFinder.findLineSegments(lineDotPass->lineDots);
  cout << "MaskWalkLineFinder   ran " << loopCount << " times. Average time: " << (Clock::getMillisSince(t)/loopCount) << " ms" << endl;

  //
  // PRINT SUMMARIES
  //
  cout << "Finished " << loopCount << " passes. Average time: " << (Clock::getMillisSince(t)/loopCount) << " ms" << endl
       << "Found:" << endl
       << "    " << lineDotPass->lineDots.size() << " line dots" << endl;

  auto blobsByLabel = blobDetectPass->getDetectedBlobs();
  for (auto const& pixelLabel : blobPixelLabels)
    cout << "    " << blobsByLabel[pixelLabel].size() << " " << pixelLabel->name() << " blob(s)" << endl;
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
  for (auto const& pixelLabel : blobPixelLabels)
  for (bold::Blob blob : blobsByLabel[pixelLabel])
  {
    auto blobColor = pixelLabel->hsvRange().toBgr()/*.invert()*/.toScalar();
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
