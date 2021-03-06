#include "localiser.ih"

Localiser::Localiser()
  : d_haveLastAgentTransform(false),
    d_lastQuaternion(0, 0, 0,0),
    d_shouldRandomise(false),
    d_pos(0, 0, 0),
    d_smoothedPos(0, 0, 0),
    d_avgPos(1)
{
  d_filterType = Config::getValue<FilterType>("localiser.filter-type");

  auto smoothingWindowSize = Config::getSetting<int>("localiser.smoothing-window-size");
//  d_useLines          = Config::getSetting<bool>("localiser.use-lines");
//  d_minGoalsNeeded    = Config::getSetting<int>("localiser.min-goals-needed");
//  auto positionError  = Config::getSetting<double>("localiser.position-error");
//  auto angleErrorDegs = Config::getSetting<double>("localiser.angle-error-degrees");
  d_defaultKidnapWeight = Config::getSetting<double>("localiser.default-kidnap-weight");
  d_penaltyKidnapWeight = Config::getSetting<double>("localiser.penalty-kidnap-weight");
  d_enablePenaltyRandomise = Config::getSetting<bool>("localiser.enable-penalty-randomise");
//  d_enableDynamicError = Config::getSetting<bool>("localiser.enable-dynamic-error");
  auto preNormWeightSumFilterAlpha = Config::getSetting<double>("localiser.prenorm-weightsum-filter-alpha");

  smoothingWindowSize->track([this](int value) { d_avgPos = MovingAverage<Vector4d>(value); });
  /*
  positionError->track([this](double value) { d_positionErrorRng = Math::createNormalRng(0, value); });
  angleErrorDegs->track([this](double value) { d_angleErrorRng = Math::createNormalRng(0, Math::degToRad(value)); });
  */
  d_preNormWeightSumFilter.reset(1.0);
  d_preNormWeightSumFilter.setAlpha(0.01);
  preNormWeightSumFilterAlpha->track([this](double value) { d_preNormWeightSumFilter.setAlpha(value); });


  double fieldXMax = (FieldMap::getFieldLengthX() + FieldMap::getOuterMarginMinimum()) / 2.0;
  double fieldYMax = (FieldMap::getFieldLengthY() + FieldMap::getOuterMarginMinimum()) / 2.0;
  d_fieldXRng = Math::createUniformRng(-fieldXMax, fieldXMax);
  d_fieldYRng = Math::createUniformRng(-fieldYMax, fieldYMax);

  double goalAreaXMin = -FieldMap::getFieldLengthX() / 2;
  double goalAreaXMax = goalAreaXMin + FieldMap::getGoalAreaLengthX();
  double goalAreaYMax = FieldMap::getGoalAreaLengthY() / 2.0;

  d_goalAreaXRng = Math::createUniformRng(goalAreaXMin, goalAreaXMax);
  d_goalAreaYRng = Math::createUniformRng(-goalAreaYMax, goalAreaYMax);

  d_thetaRng  = Math::createUniformRng(-M_PI, M_PI);

  switch (d_filterType)
  {
  case FilterType::Particle:
  {
    auto filter = allocate_aligned_shared<ParticleFilterUsed>();

    filter->setStateGenerator([this] { return this->generateState();});

    Config::getSetting<double>("localiser.randomise-ratio")->changed.connect(
      [filter](double value) {
        filter->setRandomizeRatio(value);
      });

    Config::addAction("localiser.randomize", "Randomize", [filter] { filter->randomise(); });

    Config::addAction("localiser.flip", "Flip", [this,filter]
                      {
                        // Reset the state of the smoother so we flip instantly and don't glide
                        // have our position animated slowly across the field.
                        d_avgPos.reset();

                        // Flip the x-coordinate of every particle, and flip its rotation.
                        filter->transform([](FilterState state) { return FilterState(-state.x(), state.y(), -state(2), -state(3)); });
                      });

    d_filter = filter;
    break;
  }

  case FilterType::Kalman:
  {
    d_filter = allocate_aligned_shared<KalmanFilter<4>>();
  }

  case FilterType::UnscentedKalman:
  {
    d_filter = allocate_aligned_shared<UnscentedKalmanFilter<4>>();
  }

  }

  d_filter->reset(Filter<4>::State::Zero());
  updateStateObject();
}
