#include "visualcortex.ih"

VisualCortex::VisualCortex(shared_ptr<Camera> camera,
                           shared_ptr<CameraModel> cameraModel,
                           shared_ptr<DataStreamer> dataStreamer,
                           shared_ptr<FieldMap> fieldMap,
                           shared_ptr<Spatialiser> spatialiser,
                           shared_ptr<HeadModule> headModule)
  : d_fieldMap(fieldMap),
    d_camera(camera),
    d_cameraModel(cameraModel),
    d_dataStreamer(dataStreamer),
    d_spatialiser(spatialiser),
    d_recordNextFrame(false)
{
  assert(camera);
  assert(cameraModel);
  assert(dataStreamer);
  assert(fieldMap);
  assert(spatialiser);
  assert(headModule);

  d_shouldDetectLines         = Config::getSetting<bool>("vision.line-detection.enable");
  d_shouldCountLabels         = Config::getSetting<bool>("vision.label-counter.enable");
  d_shouldDetectBlobs         = Config::getSetting<bool>("vision.blob-detection.enable");

  d_shouldIgnoreAboveHorizon  = Config::getSetting<bool>("vision.ignore-above-horizon");
  d_shouldIgnoreOutsideField  = Config::getSetting<bool>("vision.ignore-outside-field");
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
  d_shouldDrawCalibration     = Config::getSetting<bool>("round-table.image-features.calibration");
  d_shouldDrawObservedObjects = Config::getSetting<bool>("round-table.image-features.objects");

  d_ballBlobMergingEnabled     = Config::getSetting<bool>("vision.enable-ball-blob-merging");

  d_lineDotColour             = Config::getSetting<Colour::bgr>("round-table.image-colours.line-dot");
  d_observedLineColour        = Config::getSetting<Colour::bgr>("round-table.image-colours.observed-line");
  d_expectedLineColour        = Config::getSetting<Colour::bgr>("round-table.image-colours.expected-line");
  d_horizonColour             = Config::getSetting<Colour::bgr>("round-table.image-colours.horizon");
  d_fieldEdgeColour           = Config::getSetting<Colour::bgr>("round-table.image-colours.field-edge");
  d_calibrationColour         = Config::getSetting<Colour::bgr>("round-table.image-colours.calibration");

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

  d_minBallArea                   = Config::getSetting<int>("vision.min-ball-area");
  d_acceptedBallMeasuredSizeRatio = Config::getSetting<Range<double>>("vision.accepted-ball-measured-size-ratio");
  d_acceptedGoalMeasuredWidthRatio = Config::getSetting<Range<double>>("vision.accepted-goal-measured-width-ratio");
  d_minGoalDimensionPixels        = Config::getSetting<int>("vision.min-goal-dimension-pixels");
  d_maxGoalFieldEdgeDistPixels    = Config::getSetting<int>("vision.max-goal-field-edge-distance-px");

  // TODO don't pass this around -- look it up from config (?)
  int imageWidth = d_cameraModel->imageWidth();
  int imageHeight = d_cameraModel->imageHeight();

  d_imagePassHandlers = make_tuple(
    shared_ptr<LineDotPass<uchar>>(new LineDotPass<uchar>(imageWidth, d_fieldLabel, d_lineLabel)),
    shared_ptr<BlobDetectPass>(new BlobDetectPass(imageWidth, imageHeight, blobPixelLabels)),
    shared_ptr<CartoonPass>(new CartoonPass(imageWidth, imageHeight, pixelLabels)),
    shared_ptr<LabelCountPass>(new LabelCountPass(pixelLabels)),
    shared_ptr<CompleteFieldEdgePass>(new CompleteFieldEdgePass(d_fieldLabel, imageWidth, imageHeight)),
    shared_ptr<PeriodicFieldEdgePass>(new PeriodicFieldEdgePass(d_fieldLabel, imageWidth, imageHeight, 1*2*3*4))
    );

  d_imagePassRunner = shared_ptr<ImagePassRunner<uchar>>(new ImagePassRunner<uchar>());

  Config::getSetting<FieldEdgeType>("vision.field-edge-pass.field-edge-type")->track(
    [this](FieldEdgeType fieldEdgeType)
    {
      switch (fieldEdgeType)
      {
        case FieldEdgeType::Complete:
          d_imagePassRunner->removeHandler(getHandler<PeriodicFieldEdgePass>());
          d_imagePassRunner->addHandler(getHandler<CompleteFieldEdgePass>());
          d_fieldEdgePass = getHandler<CompleteFieldEdgePass>();
          break;
        case FieldEdgeType::Periodic:
          d_imagePassRunner->removeHandler(getHandler<CompleteFieldEdgePass>());
          d_imagePassRunner->addHandler(getHandler<PeriodicFieldEdgePass>());
          d_fieldEdgePass = getHandler<PeriodicFieldEdgePass>();
          break;
      }
    }
  );

  Config::getSetting<ImageGranularity>("vision.image-granularity")->track(
    [this](ImageGranularity granularity)
    {
      switch (granularity)
      {
        case ImageGranularity::All:
          d_granularityFunction = [](int y) { return Eigen::Vector2i(1,1); };
          break;
        case ImageGranularity::Half:
          d_granularityFunction = [](int y) { return Eigen::Vector2i(2,2); };
          break;
        case ImageGranularity::Third:
          d_granularityFunction = [](int y) { return Eigen::Vector2i(3,3); };
          break;
        case ImageGranularity::Gradient:
        {
          auto maxGranularity = Config::getSetting<int>("vision.max-granularity");
          d_granularityFunction = [this,maxGranularity](int y) mutable
          {
            int pixelDelta = (d_cameraModel->imageHeight() - y)/40;
            if (pixelDelta == 0)
              pixelDelta = 1;
            auto max = maxGranularity->getValue();
            if (pixelDelta > max)
              pixelDelta = max;
            return Eigen::Vector2i(pixelDelta, pixelDelta);
          };
          break;
        }
        case ImageGranularity::Projected:
        {
          auto maxGranularity = Config::getSetting<int>("vision.max-granularity");
          d_granularityFunction = [this,maxGranularity](int y) mutable
          {
            Vector2d referencePixel(d_cameraModel->imageWidth()/2, y);

            Maybe<Vector3d> midLineAgentSpace = d_spatialiser->findGroundPointForPixel(referencePixel);

            int pixelDelta = numeric_limits<int>::max();

            if (midLineAgentSpace)
            {
              double desiredDistanceMetres = 0.01; // TODO as setting
              Vector3d offsetAgentSpace(*midLineAgentSpace + Vector3d(desiredDistanceMetres, 0, 0));
              Maybe<Vector2d> offsetPixel = d_spatialiser->findPixelForAgentPoint(offsetAgentSpace);

              if (offsetPixel)
                pixelDelta = (int)round(abs((*offsetPixel - referencePixel).norm()));
            }

            auto max = maxGranularity->getValue();

            if (pixelDelta > max)
              pixelDelta = max;
            else if (pixelDelta < 1)
              pixelDelta = 1;

            return Eigen::Vector2i(pixelDelta, pixelDelta);
          };
          break;
        }
      }
    }
  );

  d_shouldDetectLines->track([this](bool value) { d_imagePassRunner->setHandler(getHandler<LineDotPass<uchar>>(), value); });
  d_shouldCountLabels->track([this](bool value) { d_imagePassRunner->setHandler(getHandler<LabelCountPass>(), value); });
  d_shouldDetectBlobs->track([this](bool value)
  {
    auto const& handler = getHandler<BlobDetectPass>();
    d_imagePassRunner->setHandler(handler, value);
    if (!value)
      handler->clear();
  });

  // Only include the cartoon pass when needed
  d_imageType->track([this](ImageType value) { d_imagePassRunner->setHandler(getHandler<CartoonPass>(), value == ImageType::Cartoon); });

  //d_lineFinder = make_shared<MaskWalkLineFinder>();
  d_lineFinder = make_shared<ScanningLineFinder>();

  // Image capture
  Config::addAction("camera.save-frame", "Save Frame", [this]() { d_recordNextFrame = true; });
}
