#include "localiser.ih"

Localiser::Localiser(shared_ptr<FieldMap> fieldMap, unsigned initialCount, double randomizeRatio)
: d_pos(0, 0, 0, 0),
  d_fieldMap(fieldMap),
  d_randomizeRatio(randomizeRatio)
{
  double xMax = (fieldMap->fieldLengthX() + fieldMap->outerMarginMinimum()) / 2.0;
  double yMax = (fieldMap->fieldLengthY() + fieldMap->outerMarginMinimum()) / 2.0;
  auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();

  uniform_real_distribution<double> fieldXDistribution(-xMax, xMax);
  uniform_real_distribution<double> fieldYDistribution(-yMax, yMax);
  uniform_real_distribution<double> thetaDistribution(-M_PI, M_PI);
  d_fieldXRng = bind(fieldXDistribution, default_random_engine(seed));
  d_fieldYRng = bind(fieldYDistribution, default_random_engine(seed + 7));
  d_thetaRng  = bind(thetaDistribution,  default_random_engine(seed + 13));

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

  auto randomizeRatioControl = Control::createInt("Randomize Ratio", int(randomizeRatio * 100), [this](int value){ d_randomizeRatio = value/100.0; });
  randomizeRatioControl.setDefaultValue(int(randomizeRatio * 100));
  randomizeRatioControl.setLimitValues(0, 100);
  d_controls.push_back(randomizeRatioControl);

  updateStateObject();
}