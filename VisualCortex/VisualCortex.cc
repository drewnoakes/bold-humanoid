#include "visualcortex.ih"

VisualCortex::VisualCortex(shared_ptr<Camera> camera,
                           shared_ptr<CameraModel> cameraModel,
                           shared_ptr<FieldMap> fieldMap,
                           shared_ptr<Spatialiser> spatialiser,
                           shared_ptr<HeadModule> headModule)
  : Configurable("visualcortex"),
    d_fieldMap(fieldMap),
    d_camera(camera),
    d_cameraModel(cameraModel),
    d_spatialiser(spatialiser),
    d_shouldIgnoreAboveHorizon(true),
    d_minBallArea(64),
    d_minGoalDimensionPixels(1),
    d_imageType(ImageType::RGB),
    d_isRecordingFrames(false),
    d_recordNextFrame(false),
    d_shouldDrawBlobs(true),
    d_shouldDrawLineDots(false),
    d_shouldDrawObservedObjects(true),
    d_shouldDrawExpectedLines(false),
    d_shouldDrawObservedLines(true),
    d_shouldDrawHorizon(true),
    d_shouldDrawFieldEdge(true)
{
  assert(camera);
  assert(cameraModel);
  assert(fieldMap);
  assert(spatialiser);
  assert(headModule);

  cout << "[VisualCortex::VisualCortex] Start" << endl;

  d_shouldDetectLines = getParam("DetectLines", 0) != 0;
  d_shouldCountLabels = getParam("CountLabels", 0) != 0;

  d_streamFramePeriod = getParam("CameraFramePeriod", 5);

  d_goalLabel =  make_shared<PixelLabel>(PixelLabel::fromConfig("Goal",   44,  60, 158, 236, 124, 222));
  d_ballLabel =  make_shared<PixelLabel>(PixelLabel::fromConfig("Ball",  248,  23,  72, 255,  12, 255));
  d_fieldLabel = make_shared<PixelLabel>(PixelLabel::fromConfig("Field",  72, 118, 166, 236,  52, 192));
  d_lineLabel =  make_shared<PixelLabel>(PixelLabel::fromConfig("Line",    0, 255,   0, 167, 161, 255));

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
  if (d_shouldCountLabels)
    d_imagePassRunner->addHandler(d_labelCountPass);

  d_lineFinder = make_shared<MaskWalkLineFinder>(imageWidth, imageHeight);

  //
  // VISION SYSTEM CONTROLS
  //

  // HeadModule control
  d_controlsByFamily["head"] = headModule->getControls();

  // Image capture controls
  vector<shared_ptr<Control const>> imageCaptureControls;
  imageCaptureControls.push_back(Control::createAction("Save Frame", [this]() { d_recordNextFrame = true; }));
  auto recordFramesControl = Control::createBool("Record Frames", [this]() { return d_isRecordingFrames; }, [this](bool value) { d_isRecordingFrames = value; });
  recordFramesControl->setIsAdvanced(true);
  imageCaptureControls.push_back(recordFramesControl);
  d_controlsByFamily["image-capture"] = imageCaptureControls;

  auto minBallAreaControl = Control::createInt("Min ball area", [this]() { return d_minBallArea; }, [this](int value) { d_minBallArea = value; });
  minBallAreaControl->setIsAdvanced(true);
  minBallAreaControl->setLimitValues(1, 100);
  auto minGoalDimensionControl = Control::createInt("Min goal dimension", [this]() { return d_minGoalDimensionPixels; }, [this](int value) { d_minGoalDimensionPixels = value; });
  minGoalDimensionControl->setIsAdvanced(true);
  minGoalDimensionControl->setLimitValues(1, 100);
  vector<shared_ptr<Control const>> objectDetectionControls = { minBallAreaControl, minGoalDimensionControl };
  d_controlsByFamily["vision/objects"] = objectDetectionControls;

  d_controlsByFamily["vision/field-edge"] = d_fieldEdgePass->getControls();

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

  vector<shared_ptr<Control const>> labelCountControls;
  labelCountControls.push_back(Control::createBool("Count labels", [this]() { return d_shouldCountLabels; }, [this](bool value)
  {
    d_shouldCountLabels = value;
    if (d_shouldCountLabels)
      d_imagePassRunner->addHandler(d_labelCountPass);
    else
      d_imagePassRunner->removeHandler(d_labelCountPass);
  }));
  d_controlsByFamily["vision/label-count"] = labelCountControls;

  // Allow control over the LUT parameters
  auto setHueMin = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withHMin(value)); createLookupTable(); };
  auto setHueMax = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withHMax(value)); createLookupTable(); };
  auto setSatMin = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withSMin(value)); createLookupTable(); };
  auto setSatMax = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withSMax(value)); createLookupTable(); };
  auto setValMin = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withVMin(value)); createLookupTable(); };
  auto setValMax = [createLookupTable](shared_ptr<PixelLabel> label, int value) { label->setHsvRange(label->hsvRange().withVMax(value)); createLookupTable(); };
  vector<shared_ptr<Control const>> lutControls;
  for (shared_ptr<PixelLabel> label : pixelLabels)
  {
    auto hMin = Control::createInt(label->name() + " Hue Min", [label]() { return label->hsvRange().hMin; }, [label,setHueMin](int value){ setHueMin(label, value); });
    auto hMax = Control::createInt(label->name() + " Hue Max", [label]() { return label->hsvRange().hMax; }, [label,setHueMax](int value){ setHueMax(label, value); });
    auto sMin = Control::createInt(label->name() + " Sat Min", [label]() { return label->hsvRange().sMin; }, [label,setSatMin](int value){ setSatMin(label, value); });
    auto sMax = Control::createInt(label->name() + " Sat Max", [label]() { return label->hsvRange().sMax; }, [label,setSatMax](int value){ setSatMax(label, value); });
    auto vMin = Control::createInt(label->name() + " Val Min", [label]() { return label->hsvRange().vMin; }, [label,setValMin](int value){ setValMin(label, value); });
    auto vMax = Control::createInt(label->name() + " Val Max", [label]() { return label->hsvRange().vMax; }, [label,setValMax](int value){ setValMax(label, value); });

    hMin->setDefaultValue(label->hsvRange().hMin);
    hMax->setDefaultValue(label->hsvRange().hMax);
    sMin->setDefaultValue(label->hsvRange().sMin);
    sMax->setDefaultValue(label->hsvRange().sMax);
    vMin->setDefaultValue(label->hsvRange().vMin);
    vMax->setDefaultValue(label->hsvRange().vMax);

    hMin->setLimitValues(0, 255);
    hMax->setLimitValues(0, 255);
    sMin->setLimitValues(0, 255);
    sMax->setLimitValues(0, 255);
    vMin->setLimitValues(0, 255);
    vMax->setLimitValues(0, 255);

    hMin->setIsAdvanced(true);
    hMax->setIsAdvanced(true);
    sMin->setIsAdvanced(true);
    sMax->setIsAdvanced(true);
    vMin->setIsAdvanced(true);
    vMax->setIsAdvanced(true);

    lutControls.push_back(hMin);
    lutControls.push_back(hMax);
    lutControls.push_back(sMin);
    lutControls.push_back(sMax);
    lutControls.push_back(vMin);
    lutControls.push_back(vMax);
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
