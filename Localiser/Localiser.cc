#include "localiser.ih"

Localiser::Localiser(shared_ptr<FieldMap> fieldMap, unsigned initialCount, double randomizeRatio)
: d_pos(0, 0, 0, 0),
  d_fieldMap(fieldMap),
  d_randomizeRatio(randomizeRatio)
{
  double xMax = (fieldMap->fieldLengthX() + fieldMap->outerMarginMinimum()) / 2.0;
  double yMax = (fieldMap->fieldLengthY() + fieldMap->outerMarginMinimum()) / 2.0;

  d_fieldXRng = Math::createUniformRng(-xMax, xMax);
  d_fieldYRng = Math::createUniformRng(-yMax, yMax);
  d_thetaRng  = Math::createUniformRng(-M_PI, M_PI);

  double initialPositionError = 0.05; // 5 cm
  unsigned initialAngleErrorDeg = 1;  // 1 degree
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

  double randomizeRatioScale = 100.0;
  auto randomizeRatioControl = Control::createInt("Randomize Ratio", int(randomizeRatio * randomizeRatioScale), [this,randomizeRatioScale](int value){ d_randomizeRatio = value/randomizeRatioScale; });
  randomizeRatioControl.setDefaultValue(int(randomizeRatio * randomizeRatioScale));
  randomizeRatioControl.setLimitValues(0 * randomizeRatioScale, 1 * randomizeRatioScale);
  d_controls.push_back(randomizeRatioControl);

  double positionErrorScale = 100.0;
  auto positionErrorControl = Control::createInt("Position Error (cm)", int(initialPositionError * positionErrorScale), [this,positionErrorScale](int value){ d_positionError = Math::createNormalRng(0, value/positionErrorScale); });
  positionErrorControl.setDefaultValue(int(initialPositionError * positionErrorScale));
  positionErrorControl.setLimitValues(0, 50);
  d_controls.push_back(positionErrorControl);

  auto angleErrorControl = Control::createInt("Angle Error (deg)", initialAngleErrorDeg, [this](int value){ d_angleError = Math::createNormalRng(0, Math::degToRad(value)); });
  angleErrorControl.setDefaultValue(initialAngleErrorDeg);
  angleErrorControl.setLimitValues(0, 20);
  d_controls.push_back(angleErrorControl);

  updateStateObject();
}