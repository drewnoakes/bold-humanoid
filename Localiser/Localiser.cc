#include "localiser.ih"

Localiser::Localiser(shared_ptr<FieldMap> fieldMap)
  : d_pos(0, 0, 0),
    d_smoothedPos(0, 0, 0),
    d_avgPos(1),
    d_fieldMap(fieldMap)
{
  auto particleCount  = Config::getSetting<int>("localiser.particle-count");
  auto smoothingWindowSize = Config::getSetting<int>("localiser.smoothing-window-size");
  d_randomizeRatio    = Config::getSetting<double>("localiser.randomise-ratio");
  d_rewardFalloff     = Config::getSetting<double>("localiser.reward-fall-off");
  d_useLines          = Config::getSetting<bool>("localiser.use-lines");
  d_minGoalsNeeded    = Config::getSetting<int>("localiser.min-goals-needed");
  auto positionError  = Config::getSetting<double>("localiser.position-error");
  auto angleErrorDegs = Config::getSetting<double>("localiser.angle-error-degrees");

  particleCount->changed.connect([this](int value) { d_filter->setParticleCount(value); });
  smoothingWindowSize->changed.connect([this](int value) { d_avgPos = MovingAverage<Vector4d>(value); });
  positionError->changed.connect([this](int value) { d_positionErrorRng = Math::createNormalRng(0, value); });
  angleErrorDegs->changed.connect([this](int value) { d_angleErrorRng = Math::createNormalRng(0, Math::degToRad(value)); });

  d_avgPos = MovingAverage<Vector4d>(smoothingWindowSize->getValue());

  double xMax = (fieldMap->fieldLengthX() + fieldMap->outerMarginMinimum()) / 2.0;
  double yMax = (fieldMap->fieldLengthY() + fieldMap->outerMarginMinimum()) / 2.0;

  d_fieldXRng = Math::createUniformRng(-xMax, xMax);
  d_fieldYRng = Math::createUniformRng(-yMax, yMax);
  d_thetaRng  = Math::createUniformRng(-M_PI, M_PI);

  d_positionErrorRng = Math::createNormalRng(0, positionError->getValue());
  d_angleErrorRng    = Math::createNormalRng(0, Math::degToRad(angleErrorDegs->getValue()));

  ParticleFilter<3>::ParticleResampler resampler =
    [this](shared_ptr<vector<Particle>> particles, unsigned particleCount)
    {
      return resample(particles, particleCount);
    };

  ParticleFilter<3>::ParticleExtractor extractor =
    [this](shared_ptr<vector<Particle>> particles) { return extract(particles); };

  d_filter = make_shared<ParticleFilter<3>>(
    particleCount->getValue(),
    [this]() { return createRandomState(); },
    resampler,
    extractor);

  //
  // Set up controls
  //

  d_controls.push_back(Control::createAction("Randomize", [this](){ d_filter->randomise(); }));

  updateStateObject();
}
