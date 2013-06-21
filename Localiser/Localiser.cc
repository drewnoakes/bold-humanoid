#include "localiser.ih"

Localiser::Localiser(shared_ptr<FieldMap> fieldMap)
  : Configurable("localiser"),
    d_pos(0, 0, 0),
    d_smoothedPos(0, 0, 0),
    d_avgPos(1),
    d_fieldMap(fieldMap)
{
  int initialCount = getParam("ParticleCount", 200);
  int smoothingWindowSize = getParam("SmoothingWindowSize", 5);
  d_randomizeRatio = getParam("RandomiseRatio", 0.05);
  d_rewardFalloff = getParam("RewardFallOff", 0.1);
  d_useLines = getParam("UseLines", 1) != 0;
  d_minGoalsNeeded = getParam("MinGoalsNeeded", 1);
  d_positionError = getParam("PositionError", 0.03);
  d_angleErrorDegs = getParam("AngleErrorDegrees", 3);

  d_avgPos = MovingAverage<Vector4d>(smoothingWindowSize);

  double xMax = (fieldMap->fieldLengthX() + fieldMap->outerMarginMinimum()) / 2.0;
  double yMax = (fieldMap->fieldLengthY() + fieldMap->outerMarginMinimum()) / 2.0;

  d_fieldXRng = Math::createUniformRng(-xMax, xMax);
  d_fieldYRng = Math::createUniformRng(-yMax, yMax);
  d_thetaRng  = Math::createUniformRng(-M_PI, M_PI);

  d_positionErrorRng = Math::createNormalRng(0, d_positionError);
  d_angleErrorRng    = Math::createNormalRng(0, Math::degToRad(d_angleErrorDegs));

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

  auto particleCountControl = Control::createInt("Particle Count", [this]() { return d_filter->getParticleCount(); }, [this](int value){ d_filter->setParticleCount(value); });
  particleCountControl->setDefaultValue(initialCount);
  particleCountControl->setLimitValues(1, 2000);
  d_controls.push_back(particleCountControl);

  auto randomizePercentControl = Control::createInt("Randomize %", [this]() { return int(d_randomizeRatio * 100); }, [this](int value){ d_randomizeRatio = value/100.0; });
  randomizePercentControl->setDefaultValue(int(d_randomizeRatio * 100));
  randomizePercentControl->setLimitValues(0, 100);
  d_controls.push_back(randomizePercentControl);

  double positionErrorScale = 1000.0;
  auto positionErrorControl = Control::createInt("Position Error (mm)", [this,positionErrorScale]() { return int(d_positionError * positionErrorScale); }, [this,positionErrorScale](int value){ d_positionError = value/positionErrorScale; d_positionErrorRng = Math::createNormalRng(0, d_positionError); });
  positionErrorControl->setDefaultValue(int(d_positionError * positionErrorScale));
  positionErrorControl->setLimitValues(0, 50);
  d_controls.push_back(positionErrorControl);

  auto angleErrorControl = Control::createInt("Angle Error (deg)", [this]() { return d_angleErrorDegs; }, [this](int value){ d_angleErrorDegs = value; d_angleErrorRng = Math::createNormalRng(0, Math::degToRad(d_angleErrorDegs)); });
  angleErrorControl->setDefaultValue(d_angleErrorDegs);
  angleErrorControl->setLimitValues(0, 20);
  d_controls.push_back(angleErrorControl);

  double falloffScale = 100.0;
  auto rewardFalloffControl = Control::createInt("Reward Falloff (x100)", [this,falloffScale]() { return int(d_rewardFalloff*falloffScale); }, [this,falloffScale](int value){ d_rewardFalloff = value/falloffScale; });
  rewardFalloffControl->setDefaultValue(int(d_rewardFalloff*falloffScale));
  rewardFalloffControl->setLimitValues(1, 20*falloffScale);
  d_controls.push_back(rewardFalloffControl);

  auto smoothingWindowSizeControl = Control::createInt("Smoothing Window", [this]() { return (int)d_avgPos.getWindowSize(); }, [this](int value){ d_avgPos = MovingAverage<Vector4d>(value); });
  smoothingWindowSizeControl->setDefaultValue(smoothingWindowSize);
  smoothingWindowSizeControl->setLimitValues(1, 100);
  d_controls.push_back(smoothingWindowSizeControl);

  d_controls.push_back(Control::createBool("Use Lines", [this]() { return d_useLines; }, [this](bool value){ d_useLines = value; }));

  auto minGoalsNeededControl = Control::createInt("Min Goals Needed", [this]() { return d_minGoalsNeeded; }, [this](int value){ d_minGoalsNeeded = value; });
  minGoalsNeededControl->setLimitValues(1, 5);
  d_controls.push_back(minGoalsNeededControl);

  updateStateObject();
}
