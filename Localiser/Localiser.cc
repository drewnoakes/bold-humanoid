#include "localiser.ih"

Localiser::Localiser(std::shared_ptr<FieldMap> fieldMap, unsigned initialCount, double randomizeRatio)
: d_pos(0, 0, 0, 0),
  d_fieldMap(fieldMap),
  d_randomizeRatio(randomizeRatio)
{
  double xMax = (fieldMap->fieldLengthX() + fieldMap->outerMarginMinimum()) / 2.0;
  double yMax = (fieldMap->fieldLengthY() + fieldMap->outerMarginMinimum()) / 2.0;
  auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();

  std::uniform_real_distribution<double> fieldXDistribution(-xMax, xMax);
  std::uniform_real_distribution<double> fieldYDistribution(-yMax, yMax);
  std::uniform_real_distribution<double> thetaDistribution(-M_PI, M_PI);
  d_fieldXRng = std::bind(fieldXDistribution, std::default_random_engine(seed));
  d_fieldYRng = std::bind(fieldYDistribution, std::default_random_engine(seed + 7));
  d_thetaRng  = std::bind(thetaDistribution,  std::default_random_engine(seed + 13));

  //bold::ParticleFilter<DIM>::ParticleFilter(
  //unsigned int,
  //bold::ParticleFilter<DIM>::StateSampler,
  //std::function<std::shared_ptr<std::vector<std::pair<Eigen::Matrix<double, DIM, 1>, double> > >(std::shared_ptr<std::vector<std::pair<Eigen::Matrix<double, DIM, 1>, double> > >)>, unsigned int) [with int DIM = 3; bold::ParticleFilter<DIM>::StateSampler = std::function<Eigen::Matrix<double, 3, 1>()>]

  ParticleFilter<3>::ParticleResampler resampler =
    [this](std::shared_ptr<std::vector<Particle>> particles, unsigned particleCount) -> std::shared_ptr<std::vector<Particle>>
    {
      return resample(particles, particleCount);
    };

  d_filter = std::make_shared<ParticleFilter<3>>(
    initialCount,
    [this]() { return createRandomState(); },
    resampler);

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

  //
  updateStateObject();
}