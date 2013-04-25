#include "localiser.ih"

Localiser::Localiser(shared_ptr<FieldMap> fieldMap, unsigned initialCount, double randomizeRatio, unsigned smoothingWindowSize, double rewardFalloff)
: d_pos(0, 0, 0),
  d_smoothedPos(0, 0, 0),
  d_avgPos(smoothingWindowSize),
  d_fieldMap(fieldMap),
  d_randomizeRatio(randomizeRatio),
  d_rewardFalloff(rewardFalloff),
  d_useLines(true),
  d_minGoalsNeeded(0)
{
  double xMax = (fieldMap->fieldLengthX() + fieldMap->outerMarginMinimum()) / 2.0;
  double yMax = (fieldMap->fieldLengthY() + fieldMap->outerMarginMinimum()) / 2.0;

  d_fieldXRng = Math::createUniformRng(-xMax, xMax);
  d_fieldYRng = Math::createUniformRng(-yMax, yMax);
  d_thetaRng  = Math::createUniformRng(-M_PI, M_PI);

  double initialPositionError = 0.03; // cm
  unsigned initialAngleErrorDeg = 3;  // degree
  d_positionError = Math::createNormalRng(0, initialPositionError);
  d_angleError    = Math::createNormalRng(0, Math::degToRad(initialAngleErrorDeg));

  ParticleFilter<3>::ParticleResampler resampler =
    [this](shared_ptr<vector<Particle>> particles, unsigned particleCount)
    {
      return resample(particles, particleCount);
    };

  ParticleFilter<3>::ParticleExtractor extractor =
    [this](shared_ptr<vector<Particle>> particles) { return extract(particles); };

  d_filter = make_shared<ParticleFilter<3>>(
    initialCount,
    [this]() { return createRandomState(); },
    resampler,
    extractor);

  //
  // Set up controls
  //

  d_controls.push_back(Control::createAction("Randomize", [this](){ d_filter->randomise(); }));

  auto particleCountControl = Control::createInt("Particle Count", initialCount, [this](int value){ d_filter->setParticleCount(value); });
  particleCountControl.setDefaultValue(initialCount);
  particleCountControl.setLimitValues(1, 2000);
  d_controls.push_back(particleCountControl);

  auto randomizePercentControl = Control::createInt("Randomize %", int(randomizeRatio * 100), [this](int value){ d_randomizeRatio = value/100.0; });
  randomizePercentControl.setDefaultValue(int(randomizeRatio * 100));
  randomizePercentControl.setLimitValues(0, 100);
  d_controls.push_back(randomizePercentControl);

  double positionErrorScale = 100.0;
  auto positionErrorControl = Control::createInt("Position Error (cm)", int(initialPositionError * positionErrorScale), [this,positionErrorScale](int value){ d_positionError = Math::createNormalRng(0, value/positionErrorScale); });
  positionErrorControl.setDefaultValue(int(initialPositionError * positionErrorScale));
  positionErrorControl.setLimitValues(0, 50);
  d_controls.push_back(positionErrorControl);

  auto angleErrorControl = Control::createInt("Angle Error (deg)", initialAngleErrorDeg, [this](int value){ d_angleError = Math::createNormalRng(0, Math::degToRad(value)); });
  angleErrorControl.setDefaultValue(initialAngleErrorDeg);
  angleErrorControl.setLimitValues(0, 20);
  d_controls.push_back(angleErrorControl);

  double falloffScale = 100.0;
  auto rewardFalloffControl = Control::createInt("Reward Falloff (x100)", int(rewardFalloff*falloffScale), [this,falloffScale](int value){ d_rewardFalloff = value/falloffScale; });
  rewardFalloffControl.setDefaultValue(int(rewardFalloff*falloffScale));
  rewardFalloffControl.setLimitValues(1, 20*falloffScale);
  d_controls.push_back(rewardFalloffControl);

  auto smoothingWindowSizeControl = Control::createInt("Smoothing Window", smoothingWindowSize, [this](int value){ d_avgPos = MovingAverage<Vector4d>(value); });
  smoothingWindowSizeControl.setDefaultValue(smoothingWindowSize);
  smoothingWindowSizeControl.setLimitValues(1, 100);
  d_controls.push_back(smoothingWindowSizeControl);

  d_controls.push_back(Control::createBool("Use Lines", d_useLines, [this](bool value){ d_useLines = value; }));

  auto minGoalsNeededControl = Control::createInt("Min Goals Needed", d_minGoalsNeeded, [this](int value){ d_minGoalsNeeded = value; });
  minGoalsNeededControl.setLimitValues(1, 5);
  d_controls.push_back(minGoalsNeededControl);

  updateStateObject();
}