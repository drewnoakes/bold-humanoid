#include "visualcortex.ih"

void VisualCortex::initialise(minIni const& ini)
{
  cout << "[VisualCortex::initialise] Initialising VisualCortex" << endl;

  d_streamFramePeriod = ini.geti("Debugger", "BroadcastFramePeriod", 5);

  d_goalLabel =  pixelLabelFromConfig(ini, "Goal",  40,  10, 210, 55, 190, 65);
  d_ballLabel =  pixelLabelFromConfig(ini, "Ball",  10,  15, 255, 95, 190, 95);
  d_fieldLabel = pixelLabelFromConfig(ini, "Field", 71,  20, 138, 55, 173, 65);
  d_lineLabel =  pixelLabelFromConfig(ini, "Line",   0, 255,   0, 70, 255, 70);

  vector<PixelLabel> pixelLabels = { d_goalLabel, d_ballLabel, d_fieldLabel, d_lineLabel };

  auto lut = LUTBuilder::buildLookUpTableYCbCr18(pixelLabels);
  d_imageLabeller = new ImageLabeller(lut);

  d_minBallArea = ini.geti("Vision", "MinBallArea", 8*8);

  auto ballUnionPred = &Run::overlaps;

  auto goalUnionPred =
    [] (Run const& a, Run const& b)
    {
      if (!Run::overlaps(a, b))
        return false;

      float ratio = (float)a.length() / (float)b.length();
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
//    d_labelCountPass
  };

  d_imagePasser = new ImagePasser<uchar>(handlers);

  d_lineFinder = new LineFinder(imageWidth, imageHeight);
};
