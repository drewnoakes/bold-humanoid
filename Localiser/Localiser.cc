#include "localiser.ih"

Localiser::Localiser(shared_ptr<FieldMap> fieldMap)
  : d_lastTranslation(0, 0, 0),
    d_lastQuaternion(0, 0, 0,0),
    d_pos(0, 0, 0),
    d_smoothedPos(0, 0, 0),
    d_avgPos(1),
    d_fieldMap(fieldMap)
{
  //auto particleCount  = Config::getSetting<int>("localiser.particle-count");
  auto smoothingWindowSize = Config::getSetting<int>("localiser.smoothing-window-size");
  d_randomizeRatio    = Config::getSetting<double>("localiser.randomise-ratio");
  d_rewardFalloff     = Config::getSetting<double>("localiser.reward-fall-off");
  d_useLines          = Config::getSetting<bool>("localiser.use-lines");
  d_minGoalsNeeded    = Config::getSetting<int>("localiser.min-goals-needed");
  auto positionError  = Config::getSetting<double>("localiser.position-error");
  auto angleErrorDegs = Config::getSetting<double>("localiser.angle-error-degrees");

  //particleCount->changed.connect([this](int value) { d_filter->setParticleCount(value); });
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

  d_filter = allocate_aligned_shared<ParticleFilter<3,50>>();
  Config::addAction("localiser.randomize", "Randomize", [this](){ d_filter->randomise(); });
  d_filter->setStateGenerator(
    [this]() {
      return ParticleFilter3::State(d_fieldXRng(), d_fieldYRng(), d_thetaRng());
    });

  Config::addAction("localiser.flip", "Flip", [this]()
  {
    // Reset the state of the smoother so we flip instantly and don't glide
    // have our position animated slowly across the field.
    d_avgPos.reset();

    // Flip the x-coordinate of every particle, and flip its rotation.
    d_filter->transform([](Vector3d state) { return Vector3d(-state.x(), state.y(), -state[2]); });
  });

  updateStateObject();
}
