#include "visualcortex.ih"

void VisualCortex::initialise(minIni const& ini)
{
  cout << "[VisualCortex::initialise] Initialising VisualCortex" << endl;

  d_goalLabel =  std::make_shared<PixelLabel>(PixelLabel::fromConfig(ini, "Goal",  40,  10, 210, 55, 190, 65));
  d_ballLabel =  std::make_shared<PixelLabel>(PixelLabel::fromConfig(ini, "Ball",  10,  15, 255, 95, 190, 95));
  d_fieldLabel = std::make_shared<PixelLabel>(PixelLabel::fromConfig(ini, "Field", 71,  20, 138, 55, 173, 65));
  d_lineLabel =  std::make_shared<PixelLabel>(PixelLabel::fromConfig(ini, "Line",   0, 255,   0, 70, 255, 70));

  vector<shared_ptr<PixelLabel>> pixelLabels = { d_goalLabel, d_ballLabel, d_fieldLabel, d_lineLabel };

  d_imageLabeller = new ImageLabeller();

  auto createLookupTable = [this,pixelLabels]()
  {
    cout << "[VisualCortex::initialise] Creating LUT using pixel labels:" << endl;
    for (shared_ptr<PixelLabel> label : pixelLabels)
      cout << "[VisualCortex::initialise]   " << label << endl;

    d_imageLabeller->updateLut(LUTBuilder::buildLookUpTableYCbCr18(pixelLabels));
  };

  createLookupTable();

  // TODO this default is probably a bit large on a 320x240 image
  d_minBallArea = ini.geti("Vision", "MinBallArea", 8*8);

  auto ballUnionPred =
    [] (Run const& a, Run const& b)
    {
      return a.overlaps(b);
    };

  auto goalUnionPred =
    [] (Run const& a, Run const& b)
    {
      if (!a.overlaps(b))
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

  //
  // Allow control over the LUT parameters
  //

  auto setHue      = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withH(value)); createLookupTable(); };
  auto setHueRange = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withHRange(value)); createLookupTable(); };
  auto setSat      = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withS(value)); createLookupTable(); };
  auto setSatRange = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withSRange(value)); createLookupTable(); };
  auto setVal      = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withV(value)); createLookupTable(); };
  auto setValRange = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withVRange(value)); createLookupTable(); };

  for (shared_ptr<PixelLabel> label : pixelLabels)
  {
    d_controls.push_back(Control::createInt(label->name() + " Hue",              label->hsvRange().h,      [label,setHue     ](int value){ setHue     (label, value); }));
    d_controls.push_back(Control::createInt(label->name() + " Hue Range",        label->hsvRange().hRange, [label,setHueRange](int value){ setHueRange(label, value); }));
    d_controls.push_back(Control::createInt(label->name() + " Saturation",       label->hsvRange().s,      [label,setSat     ](int value){ setSat     (label, value); }));
    d_controls.push_back(Control::createInt(label->name() + " Saturation Range", label->hsvRange().sRange, [label,setSatRange](int value){ setSatRange(label, value); }));
    d_controls.push_back(Control::createInt(label->name() + " Value",            label->hsvRange().v,      [label,setVal     ](int value){ setVal     (label, value); }));
    d_controls.push_back(Control::createInt(label->name() + " Value Range",      label->hsvRange().vRange, [label,setValRange](int value){ setValRange(label, value); }));
  }
};
