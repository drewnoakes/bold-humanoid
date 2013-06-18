#include "visualcortex.ih"

VisualCortex::VisualCortex(shared_ptr<CameraModel> cameraModel,
                           shared_ptr<FieldMap> fieldMap,
                           shared_ptr<Spatialiser> spatialiser,
                           shared_ptr<HeadModule> headModule)
  : Configurable("visialcortex"),
    d_fieldMap(fieldMap),
    d_cameraModel(cameraModel),
    d_spatialiser(spatialiser),
    d_shouldIgnoreAboveHorizon(true),
    d_minBallArea(64),
    d_imageType(ImageType::RGB),
    d_shouldDrawBlobs(true),
    d_shouldDrawLineDots(false),
    d_shouldDrawExpectedLines(false),
    d_shouldDrawObservedLines(true),
    d_shouldDrawHorizon(true),
    d_shouldDrawFieldEdge(true)
{
  cout << "[VisualCortex::VisualCortex] Start" << endl;

  d_shouldDetectLines = getParam("DetectLines", 1) != 0;

  d_streamFramePeriod = getParam("CameraFramePeriod", 5);

  d_goalLabel =  make_shared<PixelLabel>(PixelLabel::fromConfig("Goal",  35,  15, 209,  48, 179,  60));
  d_ballLabel =  make_shared<PixelLabel>(PixelLabel::fromConfig("Ball",   9,  20, 211,  89, 172, 138));
  d_fieldLabel = make_shared<PixelLabel>(PixelLabel::fromConfig("Field", 78,  30, 202, 106, 109, 124));
  d_lineLabel =  make_shared<PixelLabel>(PixelLabel::fromConfig("Line",   0, 255,  57, 110, 224,  63));

  vector<shared_ptr<PixelLabel>> pixelLabels = { d_ballLabel, d_goalLabel, d_fieldLabel, d_lineLabel };

  d_imageLabeller = make_shared<ImageLabeller>(d_spatialiser);

  auto createLookupTable = [this,pixelLabels]()
  {
    cout << "[VisualCortex::VisualCortex] Creating LUT using pixel labels:" << endl;
    for (shared_ptr<PixelLabel> label : pixelLabels)
      cout << "[VisualCortex::VisualCortex]   " << *label << endl;

    d_imageLabeller->updateLut(LUTBuilder::buildLookUpTableYCbCr18(pixelLabels));
  };

  createLookupTable();

  d_minBallArea = getParam("MinBallArea", 3*3);

  int imageWidth = d_cameraModel->imageWidth();
  int imageHeight = d_cameraModel->imageHeight();

  d_lineDotPass = make_shared<LineDotPass<uchar>>(imageWidth, d_fieldLabel, d_lineLabel, 3);

  vector<shared_ptr<PixelLabel>> blobPixelLabels = { d_ballLabel, d_goalLabel };

  d_blobDetectPass = make_shared<BlobDetectPass>(imageWidth, imageHeight, blobPixelLabels);
  d_cartoonPass = make_shared<CartoonPass>(imageWidth, imageHeight, pixelLabels, Colour::bgr(128,128,128));
  d_labelCountPass = make_shared<LabelCountPass>(pixelLabels);
  d_fieldEdgePass = make_shared<FieldEdgePass>(d_fieldLabel, imageWidth, imageHeight);

  d_imagePassRunner = make_shared<ImagePassRunner<uchar>>();
  
  //TODO: create control to turn this feature on and off
  d_imagePassRunner->addHandler(d_fieldEdgePass);
  
  if (d_shouldDetectLines)
    d_imagePassRunner->addHandler(d_lineDotPass);
  d_imagePassRunner->addHandler(d_blobDetectPass);
//  d_imagePassRunner->addHandler(d_cartoonPass); // will be added if a client requests cartoon images
//   d_imagePassRunner->addHandler(d_labelCountPass);

  d_lineFinder = make_shared<MaskWalkLineFinder>(imageWidth, imageHeight);
  d_controlsByFamily["lines"] = d_lineFinder->getControls();

  //
  // Allow control over the LUT parameters
  //

  auto setHue      = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withH(value));      createLookupTable(); };
  auto setHueRange = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withHRange(value)); createLookupTable(); };
  auto setSat      = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withS(value));      createLookupTable(); };
  auto setSatRange = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withSRange(value)); createLookupTable(); };
  auto setVal      = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withV(value));      createLookupTable(); };
  auto setValRange = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withVRange(value)); createLookupTable(); };

  vector<Control> lutControls;

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

    h .setIsAdvanced(true);
    hr.setIsAdvanced(true);
    s .setIsAdvanced(true);
    sr.setIsAdvanced(true);
    v .setIsAdvanced(true);
    vr.setIsAdvanced(true);

    lutControls.push_back(h);
    lutControls.push_back(hr);
    lutControls.push_back(s);
    lutControls.push_back(sr);
    lutControls.push_back(v);
    lutControls.push_back(vr);
  }

  d_controlsByFamily["lut"] = lutControls;

  vector<Control> lineDotPassControls = { d_lineDotPass->getHysterisisControl() };
  d_controlsByFamily["line-dots"] = lineDotPassControls;

  //
  // HeadModule control
  //
  d_controlsByFamily["head"] = headModule->getControls();

  //
  // Image controls
  //
  vector<Control> imageControls;
  // Image types
  vector<ControlEnumValue> imageTypes;
  imageTypes.push_back(ControlEnumValue((int)ImageType::RGB,     "RGB"));
  imageTypes.push_back(ControlEnumValue((int)ImageType::Cartoon, "Cartoon"));
  imageTypes.push_back(ControlEnumValue((int)ImageType::YCbCr,   "YCbCr"));
  imageTypes.push_back(ControlEnumValue((int)ImageType::None,    "None"));
  imageControls.push_back(
    Control::createEnum(
      "Image",
      imageTypes,
      (int)d_imageType,
      [this](ControlEnumValue const& value)
      {
        d_imageType = (ImageType)value.getValue();

        // Only include the image pass handler if we're streaming the cartoon
        if (d_imageType == ImageType::Cartoon)
          d_imagePassRunner->addHandler(d_cartoonPass);
        else
          d_imagePassRunner->removeHandler(d_cartoonPass);
      }
    )
  );

  // Frame periods
  vector<ControlEnumValue> framePeriods;
  for (int period = 1; period <= 10; period++)
  {
    framePeriods.push_back(ControlEnumValue(period, to_string(period)));
  }
  imageControls.push_back(Control::createEnum("Frame period", framePeriods, d_streamFramePeriod, [this](ControlEnumValue const& value) { d_streamFramePeriod = value.getValue(); }));

  // Layers
  // TODO: should lambdas be declared mutable?
  // TODO this should probably be in a different control family
  imageControls.push_back(Control::createBool("Ignore above horizon", d_shouldIgnoreAboveHorizon, [this](bool const& value) { d_shouldIgnoreAboveHorizon = value; }));
  
  imageControls.push_back(Control::createBool("Blobs",            d_shouldDrawBlobs,         [this](bool const& value) { d_shouldDrawBlobs = value; }));
  imageControls.push_back(Control::createBool("Line dots",        d_shouldDrawLineDots,      [this](bool const& value) { d_shouldDrawLineDots = value; }));
  imageControls.push_back(Control::createBool("Lines (observed)", d_shouldDrawObservedLines, [this](bool const& value) { d_shouldDrawObservedLines = value; }));
  imageControls.push_back(Control::createBool("Lines (expected)", d_shouldDrawExpectedLines, [this](bool const& value) { d_shouldDrawExpectedLines = value; }));
  imageControls.push_back(Control::createBool("Horizon",          d_shouldDrawHorizon,       [this](bool const& value) { d_shouldDrawHorizon = value; }));
  imageControls.push_back(Control::createBool("Field edge",       d_shouldDrawFieldEdge,     [this](bool const& value) { d_shouldDrawFieldEdge = value; }));
  d_controlsByFamily["image"] = imageControls;

  auto minBallAreaControl = Control::createInt("Min ball area", d_minBallArea, [this](int value) { d_minBallArea = value; });
  minBallAreaControl.setIsAdvanced(true);
  vector<Control> ballControls = { minBallAreaControl };
  d_controlsByFamily["ball"] = ballControls;
}
