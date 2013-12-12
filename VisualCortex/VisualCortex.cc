#include "visualcortex.ih"

VisualCortex::VisualCortex(shared_ptr<Camera> camera,
                           shared_ptr<CameraModel> cameraModel,
                           shared_ptr<FieldMap> fieldMap,
                           shared_ptr<Spatialiser> spatialiser,
                           shared_ptr<HeadModule> headModule)
  : d_fieldMap(fieldMap),
    d_camera(camera),
    d_cameraModel(cameraModel),
    d_spatialiser(spatialiser),
    d_recordNextFrame(false)
{
  assert(camera);
  assert(cameraModel);
  assert(fieldMap);
  assert(spatialiser);
  assert(headModule);

  d_shouldDetectLines         = Config::getSetting<bool>("vision.line-detection.enable");
  d_shouldCountLabels         = Config::getSetting<bool>("vision.label-counter.enable");

  d_shouldIgnoreAboveHorizon  = Config::getSetting<bool>("vision.ignore-above-horizon");
  d_isRecordingFrames         = Config::getSetting<bool>("camera.recording-frames");

  d_streamFramePeriod         = Config::getSetting<int>("round-table.camera-frame-frequency");
  d_imageType                 = Config::getSetting<ImageType>("round-table.image-type");

  d_shouldDrawBlobs           = Config::getSetting<bool>("round-table.image-features.blobs");
  d_shouldDrawLineDots        = Config::getSetting<bool>("round-table.image-features.line-dots");
  d_shouldDrawObservedLines   = Config::getSetting<bool>("round-table.image-features.observed-lines");
  d_shouldDrawExpectedLines   = Config::getSetting<bool>("round-table.image-features.expected-lines");
  d_shouldDrawExpectedLineEdges = Config::getSetting<bool>("round-table.image-features.expected-line-edges");
  d_shouldDrawHorizon         = Config::getSetting<bool>("round-table.image-features.horizon");
  d_shouldDrawFieldEdge       = Config::getSetting<bool>("round-table.image-features.field-edge");
  d_shouldDrawObservedObjects = Config::getSetting<bool>("round-table.image-features.objects");

  d_lineDotColour             = Config::getSetting<Colour::bgr>("round-table.image-colours.line-dot");
  d_observedLineColour        = Config::getSetting<Colour::bgr>("round-table.image-colours.observed-line");
  d_expectedLineColour        = Config::getSetting<Colour::bgr>("round-table.image-colours.expected-line");
  d_horizonColour             = Config::getSetting<Colour::bgr>("round-table.image-colours.horizon");
  d_fieldEdgeColour           = Config::getSetting<Colour::bgr>("round-table.image-colours.field-edge");

  d_goalLabel  = make_shared<PixelLabel>("Goal",  Config::getValue<Colour::hsvRange>("vision.pixel-labels.goal"));
  d_ballLabel  = make_shared<PixelLabel>("Ball",  Config::getValue<Colour::hsvRange>("vision.pixel-labels.ball"));
  d_fieldLabel = make_shared<PixelLabel>("Field", Config::getValue<Colour::hsvRange>("vision.pixel-labels.field"));
  d_lineLabel  = make_shared<PixelLabel>("Line",  Config::getValue<Colour::hsvRange>("vision.pixel-labels.line"));

  vector<shared_ptr<PixelLabel>> pixelLabels = { d_ballLabel, d_goalLabel, d_fieldLabel, d_lineLabel };
  vector<shared_ptr<PixelLabel>> blobPixelLabels = { d_ballLabel, d_goalLabel };

  d_imageLabeller = make_shared<ImageLabeller>(d_spatialiser);

  auto createLookupTable = [this,pixelLabels]()
  {
    log::info("VisualCortex::VisualCortex") << "Creating LUT using pixel labels:";
    for (shared_ptr<PixelLabel> label : pixelLabels)
      log::info("VisualCortex::VisualCortex") << "  " << *label;

    d_imageLabeller->updateLut(LUTBuilder::buildLookUpTableYCbCr18(pixelLabels));
  };

  createLookupTable();

  // Recreate lookup table on dynamic configuration changes
  Config::getSetting<Colour::hsvRange>("vision.pixel-labels.goal") ->changed.connect([this,createLookupTable](Colour::hsvRange value) { d_goalLabel ->setHsvRange(value); createLookupTable(); });
  Config::getSetting<Colour::hsvRange>("vision.pixel-labels.ball") ->changed.connect([this,createLookupTable](Colour::hsvRange value) { d_ballLabel ->setHsvRange(value); createLookupTable(); });
  Config::getSetting<Colour::hsvRange>("vision.pixel-labels.field")->changed.connect([this,createLookupTable](Colour::hsvRange value) { d_fieldLabel->setHsvRange(value); createLookupTable(); });
  Config::getSetting<Colour::hsvRange>("vision.pixel-labels.line") ->changed.connect([this,createLookupTable](Colour::hsvRange value) { d_lineLabel ->setHsvRange(value); createLookupTable(); });

  d_minBallArea            = Config::getSetting<int>("vision.min-ball-area");
  d_minGoalDimensionPixels = Config::getSetting<int>("vision.min-goal-dimension-pixels");

  // TODO don't pass this around -- look it up from config (?)
  int imageWidth = d_cameraModel->imageWidth();
  int imageHeight = d_cameraModel->imageHeight();

  d_lineDotPass = make_shared<LineDotPass<uchar>>(imageWidth, d_fieldLabel, d_lineLabel);
  d_blobDetectPass = make_shared<BlobDetectPass>(imageWidth, imageHeight, blobPixelLabels);
  d_cartoonPass = make_shared<CartoonPass>(imageWidth, imageHeight, pixelLabels, Colour::bgr(128,128,128));
  d_labelCountPass = make_shared<LabelCountPass>(pixelLabels);
  d_fieldEdgePass = make_shared<FieldEdgePass>(d_fieldLabel, imageWidth, imageHeight);

  d_imagePassRunner = make_shared<ImagePassRunner<uchar>>();

  d_shouldDetectLines->track([this](bool value) { d_imagePassRunner->setHandler(d_lineDotPass, value); });
  d_shouldCountLabels->track([this](bool value) { d_imagePassRunner->setHandler(d_labelCountPass, value); });

  // TODO SETTINGS create a setting to turn this feature on and off
  d_imagePassRunner->addHandler(d_fieldEdgePass);
  d_imagePassRunner->addHandler(d_blobDetectPass);

  // Only include the cartoon pass when needed
  d_imageType->track([this](ImageType value) { d_imagePassRunner->setHandler(d_cartoonPass, value == ImageType::Cartoon); });

  d_lineFinder = make_shared<MaskWalkLineFinder>();

  // Image capture
  Config::addAction("camera.save-frame", "Save Frame", [this]() { d_recordNextFrame = true; });
}
