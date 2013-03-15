#include "visualcortex.ih"

void VisualCortex::initialise(minIni const& ini)
{
  cout << "[VisualCortex::initialise] Start" << endl;

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
      cout << "[VisualCortex::initialise]   " << *label << endl;

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

  d_imagePassRunner = new ImagePassRunner<uchar>(handlers);

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
    Control h  = Control::createInt(label->name() + " Hue",              label->hsvRange().h,      [label,setHue     ](int value){ setHue     (label, value); });
    Control hr = Control::createInt(label->name() + " Hue Range",        label->hsvRange().hRange, [label,setHueRange](int value){ setHueRange(label, value); });
    Control s  = Control::createInt(label->name() + " Saturation",       label->hsvRange().s,      [label,setSat     ](int value){ setSat     (label, value); });
    Control sr = Control::createInt(label->name() + " Saturation Range", label->hsvRange().sRange, [label,setSatRange](int value){ setSatRange(label, value); });
    Control v  = Control::createInt(label->name() + " Value",            label->hsvRange().v,      [label,setVal     ](int value){ setVal     (label, value); });
    Control vr = Control::createInt(label->name() + " Value Range",      label->hsvRange().vRange, [label,setValRange](int value){ setValRange(label, value); });

    h .setDefaultValue(label->hsvRange().h);
    hr.setDefaultValue(label->hsvRange().hRange);
    s .setDefaultValue(label->hsvRange().s);
    sr.setDefaultValue(label->hsvRange().sRange);
    v .setDefaultValue(label->hsvRange().v);
    vr.setDefaultValue(label->hsvRange().vRange);

    h .setLimitValues(0, 255);
    hr.setLimitValues(0, 255);
    s .setLimitValues(0, 255);
    sr.setLimitValues(0, 255);
    v .setLimitValues(0, 255);
    vr.setLimitValues(0, 255);

    d_controls.push_back(h);
    d_controls.push_back(hr);
    d_controls.push_back(s);
    d_controls.push_back(sr);
    d_controls.push_back(v);
    d_controls.push_back(vr);
  }
};
