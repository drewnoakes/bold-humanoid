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
    d_minGoalDimensionPixels(1),
    d_fieldEdgeSmoothingWindow(15),
    d_imageType(ImageType::RGB),
    d_shouldDrawBlobs(true),
    d_shouldDrawLineDots(false),
    d_shouldDrawObservedObjects(true),
    d_shouldDrawExpectedLines(false),
    d_shouldDrawObservedLines(true),
    d_shouldDrawHorizon(true),
    d_shouldDrawFieldEdge(true)
{
  cout << "[VisualCortex::VisualCortex] Start" << endl;

  d_shouldDetectLines = getParam("DetectLines", 0) != 0;

  d_streamFramePeriod = getParam("CameraFramePeriod", 5);

  d_goalLabel =  make_shared<PixelLabel>(PixelLabel::fromConfig("Goal",  52,   8, 197,  39, 173,  49));
  d_ballLabel =  make_shared<PixelLabel>(PixelLabel::fromConfig("Ball",   8,  15, 168,  96, 157, 145));
  d_fieldLabel = make_shared<PixelLabel>(PixelLabel::fromConfig("Field", 95,  23, 201,  35, 122,  70));
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
  d_minGoalDimensionPixels = getParam("MinGoalDimensionPixels", 1);
  d_fieldEdgeSmoothingWindow = getParam("FieldEdgeSmoothingWindow", 15);

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

  d_lineFinder = make_shared<MaskWalkLineFinder>(imageWidth, imageHeight);

  // HeadModule control
  d_controlsByFamily["head"] = headModule->getControls();

  //
  // VISION SYSTEM CONTROLS
  //

  auto minBallAreaControl = Control::createInt("Min ball area", [this]() { return d_minBallArea; }, [this](int value) { d_minBallArea = value; });
  minBallAreaControl->setIsAdvanced(true);
  minBallAreaControl->setLimitValues(1, 100);
  auto minGoalDimensionControl = Control::createInt("Min goal dimension", [this]() { return d_minGoalDimensionPixels; }, [this](int value) { d_minGoalDimensionPixels = value; });
  minGoalDimensionControl->setIsAdvanced(true);
  minGoalDimensionControl->setLimitValues(1, 100);
  vector<shared_ptr<Control const>> objectDetectionControls = { minBallAreaControl, minGoalDimensionControl };
  d_controlsByFamily["vision/objects"] = objectDetectionControls;

  auto fieldEdgeSmoothWindowSize = Control::createInt("Field edge smooth window size", [this]() { return d_fieldEdgeSmoothingWindow; }, [this](int value) { d_fieldEdgeSmoothingWindow = value; });
  fieldEdgeSmoothWindowSize->setIsAdvanced(true);
  fieldEdgeSmoothWindowSize->setLimitValues(1, 100);
  vector<shared_ptr<Control const>> fieldEdgeControls = { fieldEdgeSmoothWindowSize };
  d_controlsByFamily["vision/field-edge"] = fieldEdgeControls;

  vector<shared_ptr<Control const>> lineDetectionControls;
  for (auto c : d_lineFinder->getControls())
    lineDetectionControls.push_back(c);
  lineDetectionControls.push_back(Control::createBool("Detect lines", [this]() { return d_shouldDetectLines; }, [this](bool value)
  {
    d_shouldDetectLines = value;
    if (d_shouldDetectLines)
      d_imagePassRunner->addHandler(d_lineDotPass);
    else
      d_imagePassRunner->removeHandler(d_lineDotPass);
  }));
  for (auto c : d_lineDotPass->getControls())
    lineDetectionControls.push_back(c);
  d_controlsByFamily["vision/line-detection"] = lineDetectionControls;

  // Allow control over the LUT parameters
  auto setHue      = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withH(value));      createLookupTable(); };
  auto setHueRange = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withHRange(value)); createLookupTable(); };
  auto setSat      = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withS(value));      createLookupTable(); };
  auto setSatRange = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withSRange(value)); createLookupTable(); };
  auto setVal      = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withV(value));      createLookupTable(); };
  auto setValRange = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withVRange(value)); createLookupTable(); };
  vector<shared_ptr<Control const>> lutControls;
  for (shared_ptr<PixelLabel> label : pixelLabels)
  {
    auto h  = Control::createInt(label->name() + " Hue",              [label]() { return label->hsvRange().h; },      [label,setHue     ](int value){ setHue     (label, value); });
    auto hr = Control::createInt(label->name() + " Hue Range",        [label]() { return label->hsvRange().hRange; }, [label,setHueRange](int value){ setHueRange(label, value); });
    auto s  = Control::createInt(label->name() + " Saturation",       [label]() { return label->hsvRange().s; },      [label,setSat     ](int value){ setSat     (label, value); });
    auto sr = Control::createInt(label->name() + " Saturation Range", [label]() { return label->hsvRange().sRange; }, [label,setSatRange](int value){ setSatRange(label, value); });
    auto v  = Control::createInt(label->name() + " Value",            [label]() { return label->hsvRange().v; },      [label,setVal     ](int value){ setVal     (label, value); });
    auto vr = Control::createInt(label->name() + " Value Range",      [label]() { return label->hsvRange().vRange; }, [label,setValRange](int value){ setValRange(label, value); });

    h ->setDefaultValue(label->hsvRange().h);
    hr->setDefaultValue(label->hsvRange().hRange);
    s ->setDefaultValue(label->hsvRange().s);
    sr->setDefaultValue(label->hsvRange().sRange);
    v ->setDefaultValue(label->hsvRange().v);
    vr->setDefaultValue(label->hsvRange().vRange);

    h ->setLimitValues(0, 255);
    hr->setLimitValues(0, 255);
    s ->setLimitValues(0, 255);
    sr->setLimitValues(0, 255);
    v ->setLimitValues(0, 255);
    vr->setLimitValues(0, 255);

    h ->setIsAdvanced(true);
    hr->setIsAdvanced(true);
    s ->setIsAdvanced(true);
    sr->setIsAdvanced(true);
    v ->setIsAdvanced(true);
    vr->setIsAdvanced(true);

    lutControls.push_back(h);
    lutControls.push_back(hr);
    lutControls.push_back(s);
    lutControls.push_back(sr);
    lutControls.push_back(v);
    lutControls.push_back(vr);
  }

  d_controlsByFamily["vision/lut"] = lutControls;

  vector<shared_ptr<Control const>> horizonControls = { Control::createBool("Ignore above horizon", [this]() { return d_shouldIgnoreAboveHorizon; }, [this](bool const& value) { d_shouldIgnoreAboveHorizon = value; }) };
  d_controlsByFamily["vision/horizon"] = horizonControls;

  //
  // DEBUG IMAGE CONTROLS
  //

  vector<shared_ptr<Control const>> debugImageControls;
  // Image types
  vector<ControlEnumValue> imageTypes;
  imageTypes.push_back(ControlEnumValue((int)ImageType::RGB,     "RGB"));
  imageTypes.push_back(ControlEnumValue((int)ImageType::Cartoon, "Cartoon"));
  imageTypes.push_back(ControlEnumValue((int)ImageType::YCbCr,   "YCbCr"));
  imageTypes.push_back(ControlEnumValue((int)ImageType::None,    "None"));
  debugImageControls.push_back(
    Control::createEnum(
      "Image",
      imageTypes,
      [this]() { return (int)d_imageType; },
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
  debugImageControls.push_back(Control::createEnum("Frame period", framePeriods, [this]() { return d_streamFramePeriod; }, [this](ControlEnumValue const& value) { d_streamFramePeriod = value.getValue(); }));
  d_controlsByFamily["debug-image"] = debugImageControls;

  // Layers
  vector<shared_ptr<Control const>> debugImageFeaturesControls;
  debugImageFeaturesControls.push_back(Control::createBool("Blobs",            [this]() { return d_shouldDrawBlobs; },           [this](bool const& value) { d_shouldDrawBlobs = value; }));
  debugImageFeaturesControls.push_back(Control::createBool("Line dots",        [this]() { return d_shouldDrawLineDots; },        [this](bool const& value) { d_shouldDrawLineDots = value; }));
  debugImageFeaturesControls.push_back(Control::createBool("Lines (observed)", [this]() { return d_shouldDrawObservedLines; },   [this](bool const& value) { d_shouldDrawObservedLines = value; }));
  debugImageFeaturesControls.push_back(Control::createBool("Lines (expected)", [this]() { return d_shouldDrawExpectedLines; },   [this](bool const& value) { d_shouldDrawExpectedLines = value; }));
  debugImageFeaturesControls.push_back(Control::createBool("Horizon",          [this]() { return d_shouldDrawHorizon; },         [this](bool const& value) { d_shouldDrawHorizon = value; }));
  debugImageFeaturesControls.push_back(Control::createBool("Field edge",       [this]() { return d_shouldDrawFieldEdge; },       [this](bool const& value) { d_shouldDrawFieldEdge = value; }));
  debugImageFeaturesControls.push_back(Control::createBool("Objects",          [this]() { return d_shouldDrawObservedObjects; }, [this](bool const& value) { d_shouldDrawObservedObjects = value; }));
  d_controlsByFamily["debug-image-features"] = debugImageFeaturesControls;
}
