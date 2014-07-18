#include <iostream>
#include <time.h>
#include <sys/time.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "../CameraModel/cameramodel.hh"
#include "../Clock/clock.hh"
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
#include "../LineFinder/ScanningLineFinder/scanninglinefinder.hh"
#include "../LUTBuilder/lutbuilder.hh"
#include "../Painter/painter.hh"
#include "../PixelFilterChain/pixelfilterchain.hh"
#include "../PixelLabel/RangePixelLabel/rangepixellabel.hh"
#include "../PixelLabel/HistogramPixelLabel/histogrampixellabel.hh"
#include "../SequentialTimer/sequentialtimer.hh"
#include "../util/meta.hh"


using namespace cv;
using namespace std;
using namespace bold;
using namespace Eigen;

/*
void doHistogramPixels(cv::Mat const& img)
{
  HistogramPixelLabel<5> label{"label"};


  for (int y = 0; y < img.rows; ++y)
  {
    auto const* row = img.ptr<uint8_t>(y);
    for (int x = 0; x < img.cols; x += img.channels())
    {
      Colour::bgr bgr;
      bgr.b = row[x + 0];
      bgr.g = row[x + 1];
      bgr.r = row[x + 2];

      Colour::hsv hsv = Colour::bgr2hsv(bgr);
      label.addSample(hsv);
    }
  }

/ *
  auto rng = Math::createNormalRng(128, 64);

  for (unsigned i = 0; i < 1000000; ++i)
  {
    Colour::hsv hsv;
    hsv.h = rng();
    hsv.s = rng();
    hsv.v = rng();
    label.addSample(hsv);
  }
* /

  auto hsImg = label.getHSImage();
  cv::imwrite("hsimg.png", hsImg);
  auto hvImg = label.getHVImage();
  cv::imwrite("hvimg.png", hvImg);
  auto svImg = label.getSVImage();
  cv::imwrite("svimg.png", svImg);
}

*/

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    cout <<" Usage: passtest <rgb-image> [configuration]" << endl;
    return -1;
  }

  string configurationFile = "configuration.json";
  if (argc > 2)
    configurationFile = string(argv[2]);
  cout << "Using config file: " << configurationFile << endl;

  Config::initialise("configuration-metadata.json", configurationFile);

  int loopCount = 100;

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

  //doHistogramPixels(colourImage);

  // Initialise random seed
  std::srand(unsigned(std::time(0)));

  int imageWidth = colourImage.cols;
  int imageHeight = colourImage.rows;

  //
  // FIXED START UP INITIALISATION
  //

  auto t = Clock::getTimestamp();

  // Build colour ranges for segmentation
  auto goalLabel  = make_shared<RangePixelLabel>("Goal",  LabelClass::GOAL, Config::getValue<Colour::hsvRange>("vision.pixel-labels.goal"));
  auto ballLabel  = make_shared<RangePixelLabel>("Ball",  LabelClass::BALL, Config::getValue<Colour::hsvRange>("vision.pixel-labels.ball"));
  auto fieldLabel = make_shared<RangePixelLabel>("Field", LabelClass::FIELD, Config::getValue<Colour::hsvRange>("vision.pixel-labels.field"));
  auto lineLabel  = make_shared<RangePixelLabel>("Line",  LabelClass::LINE, Config::getValue<Colour::hsvRange>("vision.pixel-labels.line"));

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

  const vector<shared_ptr<PixelLabel>> blobPixelLabels = { ballLabel, goalLabel, lineLabel };
  auto blobDetectPass = make_shared<BlobDetectPass>(imageWidth, imageHeight, blobPixelLabels);

  // Resources for finding line dots
  auto lineDotPass = make_shared<LineDotPass<uchar>>(imageWidth, fieldLabel, lineLabel);

  // Resources for creating a labelled image
  auto cartoonPass = make_shared<CartoonPass>(imageWidth, imageHeight, labels);

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

  MaskWalkLineFinder maskWalkLineFinder;

  auto cameraModel = allocate_aligned_shared<CameraModel>();
  ScanningLineFinder scanningLineFinder(cameraModel);

  cout << "Startup took " << Clock::getMillisSince(t) << " ms" << endl;

  //
  // IMAGE LABELLING
  //
  t = Clock::getTimestamp();
  auto granularityFunction = [](int y) { return Vector2i(1, 1); };
  for (int i = 0; i < loopCount; i++)
  {
    SequentialTimer timer;
    imageLabeller->label(colourImage, labelledImage, timer, granularityFunction);
  }
  cout << "Labelled " << loopCount << " times. Average time: " << (Clock::getMillisSince(t)/loopCount) << " ms" << endl;

  //
  // IMAGE PASS
  //
  SequentialTimer timer;

  t = Clock::getTimestamp();
  for (int i = 0; i < loopCount; i++)
  {
    passRunner.pass(labelledImage, granularityFunction, timer);
  }
  cout << "[simple pass] Passed " << loopCount << " times. Average time: " << (Clock::getMillisSince(t)/loopCount) << " ms" << endl;

  t = Clock::getTimestamp();
  for (int i = 0; i < loopCount; i++)
  {
    passRunner.passWithHandler(lineDotPass, labelledImage, granularityFunction, timer);
    passRunner.passWithHandler(blobDetectPass, labelledImage, granularityFunction, timer);
    passRunner.passWithHandler(cartoonPass, labelledImage, granularityFunction, timer);
    passRunner.passWithHandler(labelCountPass, labelledImage, granularityFunction, timer);
  }
  cout << "[direct pass] Passed " << loopCount << " times. Average time: " << (Clock::getMillisSince(t)/loopCount) << " ms" << endl;

  auto passTuple = make_tuple(lineDotPass, blobDetectPass, cartoonPass, labelCountPass);
  t = Clock::getTimestamp();
  for (int i = 0; i < loopCount; i++)
  {
    passRunner.passWithHandlers(passTuple, labelledImage, granularityFunction, timer);
  }
  cout << "[meta pass] Passed " << loopCount << " times. Average time: " << (Clock::getMillisSince(t)/loopCount) << " ms" << endl;

  //
  // DETECT BLOBS
  //
  t = Clock::getTimestamp();
  for (int i = 0; i < loopCount; i++)
    blobDetectPass->detectBlobs(timer);
  cout << "BlobDetectPass::detectBlobs ran " << loopCount << " times. Average time: " << (Clock::getMillisSince(t)/loopCount) << " ms" << endl;
  

  //
  // FIND LINES (RandomPairLineFinder)
  //
  t = Clock::getTimestamp();
  vector<LineSegment2i> randomPairLines;
  for (int i = 0; i < loopCount; i++)
    randomPairLines = randomPairLineFinder.findLineSegments(lineDotPass->lineDots);
  cout << "RandomPairLineFinder ran " << loopCount << " times. Average time: " << (Clock::getMillisSince(t)/loopCount) << " ms" << endl;

  //
  // FIND LINES (MaskWalkLineFinder)
  //
  t = Clock::getTimestamp();
  vector<LineSegment2i> maskWalkLines;
  for (int i = 0; i < loopCount; i++)
    maskWalkLines = maskWalkLineFinder.findLineSegments(lineDotPass->lineDots);
  cout << "MaskWalkLineFinder   ran " << loopCount << " times. Average time: " << (Clock::getMillisSince(t)/loopCount) << " ms" << endl;

  //
  // FIND LINES (ScanningLineFinder)
  //
  t = Clock::getTimestamp();
  vector<LineSegment2i> scanningLines;
  for (int i = 0; i < loopCount; i++)
    scanningLines = scanningLineFinder.findLineSegments(lineDotPass->lineDots);
  cout << "ScanningLineFinder   ran " << loopCount << " times. Average time: " << (Clock::getMillisSince(t)/loopCount) << " ms" << endl;


  //
  // PRINT SUMMARIES
  //
  cout << "Finished " << loopCount << " passes. Average time: " << (Clock::getMillisSince(t)/loopCount) << " ms" << endl
       << "Found:" << endl
       << "    " << lineDotPass->lineDots.size() << " line dots" << endl;

  auto blobsByLabel = blobDetectPass->getDetectedBlobs();
  for (auto const& pixelLabel : blobPixelLabels)
    cout << "    " << blobsByLabel[pixelLabel].size() << " " << pixelLabel->getName() << " blob(s)" << endl;
  //for (auto const& pair : labelCountPass->getCounts())
  //  cout << "    " << pair.second << " " << pair.first->getName() << " pixels" << endl;

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

  // Draw line segments
//   vector<LineSegment2i> segments = lineFinder.find(lineDotPass->lineDots);
//   cout << "    " << segments.size() << " line segments" << endl;
   for (LineSegment2i const& segment : scanningLines)
   {
     cout << segment << endl;
     Painter::draw(segment, colourImage, Colour::bgr(192,0,0), 2);
   }

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
    //line.draw(colourImage, colours[colourIndex++ % colours.size()], 2);
  }
  cout << "    " << scanningLines.size() << " line(s) via ScanningLineFinder" << endl;
  for (LineSegment2i const& line : scanningLines)
  {
    cout << "      " << line << endl;
    //line.draw(colourImage, colours[colourIndex++ % colours.size()], 2);
  }

  // Draw blobs
  for (auto const& pixelLabel : blobPixelLabels)
    for (bold::Blob blob : blobsByLabel[pixelLabel])
    {
      auto blobColor = pixelLabel->modalColour().toBgr()/*.invert()*/.toScalar();
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
