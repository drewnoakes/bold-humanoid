#include "localiser.ih"

Localiser::Localiser(std::shared_ptr<FieldMap> fieldMap)
: d_pos(-2.7, 0, 0.23, 0)
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

  auto samplerFactory = std::make_shared<WheelSamplerFactory<3>>();

  auto randomState = [this]() -> ParticleFilter<3>::State
  {
    // generate an initial random state
    // TODO can we use the game mode to bias the randomness? eg before kickoff, will be on a known side, or nearer prior known locations
    return ParticleFilter<3>::State(d_fieldXRng(), d_fieldYRng(), d_thetaRng());
  };

  d_filter = std::make_shared<ParticleFilter<3>>(200, randomState, samplerFactory);

  updateStateObject();
}