#include "localiser.ih"

Localiser::Localiser(shared_ptr<FieldMap> fieldMap)
  : d_lastTranslation(0, 0, 0),
    d_lastQuaternion(0, 0, 0,0),
    d_pos(0, 0, 0),
    d_smoothedPos(0, 0, 0),
    d_avgPos(1),
    d_fieldMap(fieldMap)
{
  auto smoothingWindowSize = Config::getSetting<int>("localiser.smoothing-window-size");
  d_useLines          = Config::getSetting<bool>("localiser.use-lines");
  d_minGoalsNeeded    = Config::getSetting<int>("localiser.min-goals-needed");
  auto positionError  = Config::getSetting<double>("localiser.position-error");
  auto angleErrorDegs = Config::getSetting<double>("localiser.angle-error-degrees");

  Config::getSetting<double>("localiser.randomise-ratio")->changed.connect([this](double value) { d_filter->setRandomizeRatio(value); });

  smoothingWindowSize->track([this](int value) { d_avgPos = MovingAverage<Vector4d>(value); });
  positionError->track([this](double value) { d_positionErrorRng = Math::createNormalRng(0, value); });
  angleErrorDegs->track([this](double value) { d_angleErrorRng = Math::createNormalRng(0, Math::degToRad(value)); });

  double xMax = (fieldMap->fieldLengthX() + fieldMap->outerMarginMinimum()) / 2.0;
  double yMax = (fieldMap->fieldLengthY() + fieldMap->outerMarginMinimum()) / 2.0;

  d_fieldXRng = Math::createUniformRng(-xMax, xMax);
  d_fieldYRng = Math::createUniformRng(-yMax, yMax);
  d_thetaRng  = Math::createUniformRng(-M_PI, M_PI);

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
