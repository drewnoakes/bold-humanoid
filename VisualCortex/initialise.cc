#include "visualcortex.hh"
#include <ImagePassHandler/BlobDetectPass/blobdetectpass.hh>
#include <ImagePassHandler/LineDotPass/linedotpass.hh>
#include <ImagePasser/imagepasser.hh>
#include <LineFinder/linefinder.hh>

using namespace cv;
using namespace bold;
using namespace std;
using namespace Eigen;

#include <functional>

void VisualCortex::initialise(minIni const& ini)
{
  cout << "[VisualCortex::initialise] Initialising VisualCortex" << endl;

  d_pfChain.pushFilter([](unsigned char* pxl) {
    int y = pxl[0] - 16;
    int cb = pxl[1] - 128;
    int cr = pxl[2] - 128;

    int b = (298 * y + 516 * cb + 128) >> 8;
    if (b < 0)
      b = 0;
    int g = (298 * y - 100 * cb - 208 * cr) >> 8;
    if (g < 0)
      g = 0;
    int r = (298 * y + 409 * cr + 128) >> 8;
    if (r < 0)
      r = 0;

    pxl[0] = b;
    pxl[1] = g;
    pxl[2] = r;
  });

  d_goalLabel =  pixelLabelFromConfig(ini, "Goal",  40,  10, 210, 55, 190, 65);
  d_ballLabel =  pixelLabelFromConfig(ini, "Ball",  10,  15, 255, 95, 190, 95);
  d_fieldLabel = pixelLabelFromConfig(ini, "Field", 71,  20, 138, 55, 173, 65);
  d_lineLabel =  pixelLabelFromConfig(ini, "Line",   0, 255,   0, 70, 255, 70);

  vector<PixelLabel> pixelLabels = { d_goalLabel, d_ballLabel, d_fieldLabel, d_lineLabel };

  d_imageLabeller = new ImageLabeller(pixelLabels);

  d_minBallArea = ini.geti("Vision", "MinBallArea", 8*8);

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

  int imageWidth = 320; // TODO source image width/height from config
  int imageHeight = 240;

  d_lineDotPass = new LineDotPass<uchar>(imageWidth, d_fieldLabel, d_lineLabel, 3);

  vector<BlobType> blobTypes = {
    BlobType(d_ballLabel, ballUnionPred),
    BlobType(d_goalLabel, goalUnionPred)
  };

  d_blobDetectPass = new BlobDetectPass(imageWidth, imageHeight, blobTypes);
  d_cartoonPass = new CartoonPass(imageWidth, imageHeight, pixelLabels, Colour::bgr(128,128,128));
  d_labelCountPass = new LabelCountPass(pixelLabels);

  vector<ImagePassHandler<uchar>*> handlers = {
    d_lineDotPass,
    d_blobDetectPass,
    d_cartoonPass,
    d_labelCountPass
  };

  d_imagePasser = new ImagePasser<uchar>(handlers);

  d_lineFinder = new LineFinder(imageWidth, imageHeight);
};
