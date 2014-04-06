#include "localiser.ih"

Localiser::Localiser()
  : d_haveLastAgentTransform(false),
    d_lastQuaternion(0, 0, 0,0),
    d_pos(0, 0, 0),
    d_smoothedPos(0, 0, 0),
    d_avgPos(1)
{
  d_filterType = Config::getValue<FilterType>("localiser.filter-type");

  auto smoothingWindowSize = Config::getSetting<int>("localiser.smoothing-window-size");
  d_useLines          = Config::getSetting<bool>("localiser.use-lines");
  d_minGoalsNeeded    = Config::getSetting<int>("localiser.min-goals-needed");
  auto positionError  = Config::getSetting<double>("localiser.position-error");
  auto angleErrorDegs = Config::getSetting<double>("localiser.angle-error-degrees");
  d_defaultKidnapWeight = Config::getSetting<double>("localiser.default-kidnap-weight");
  d_penaltyKidnapWeight = Config::getSetting<double>("localiser.penalty-kidnap-weight");

  smoothingWindowSize->track([this](int value) { d_avgPos = MovingAverage<Vector4d>(value); });
  positionError->track([this](double value) { d_positionErrorRng = Math::createNormalRng(0, value); });
  angleErrorDegs->track([this](double value) { d_angleErrorRng = Math::createNormalRng(0, Math::degToRad(value)); });

  double xMax = (FieldMap::fieldLengthX() + FieldMap::outerMarginMinimum()) / 2.0;
  double yMax = (FieldMap::fieldLengthY() + FieldMap::outerMarginMinimum()) / 2.0;

  d_fieldXRng = Math::createUniformRng(-xMax, xMax);
  d_fieldYRng = Math::createUniformRng(-yMax, yMax);
  d_thetaRng  = Math::createUniformRng(-M_PI, M_PI);

  switch (d_filterType)
  {
  case FilterType::Particle:
  {
    auto filter = allocate_aligned_shared<ParticleFilterUsed>();
    
    filter->setStateGenerator(
      [this]() {
      auto theta = d_thetaRng();
      auto state = FilterState(d_fieldXRng(), d_fieldYRng(), cos(theta), sin(theta));

      // Weight should be probability of being kidnapped
      // (Weight of other particles should actually be multiplied by 1 - this probability)
      auto gameState = State::get<GameState>();
      
      auto kidnapWeight =
        gameState->myPlayerInfo().hasPenalty() ? 
        d_penaltyKidnapWeight->getValue() : 
        d_defaultKidnapWeight->getValue();

      return make_pair(state, kidnapWeight);
    });

    Config::getSetting<double>("localiser.randomise-ratio")->changed.connect(
      [filter](double value) {
        filter->setRandomizeRatio(value);
      });

    Config::addAction("localiser.randomize", "Randomize", [filter](){ filter->randomise(); });

    Config::addAction("localiser.flip", "Flip", [this,filter]()
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
  }

  updateStateObject();
}
