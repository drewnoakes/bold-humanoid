#ifndef BOLD_LOCALISER_HH
#define BOLD_LOCALISER_HH

#include <memory>

#include "../AgentPosition/agentposition.hh"
#include "../AgentState/agentstate.hh"
#include "../Filter/ParticleFilter/particlefilter.hh"
#include "../ParticleSamplerFactory/WheelSamplerFactory/wheelsamplerfactory.hh"
#include "../StateObject/ParticleState/particlestate.hh"
#include "../StateObject/BodyState/bodystate.hh"
#include "../FieldMap/fieldmap.hh"

namespace bold
{
  class Localiser
  {
  public:
    Localiser(std::shared_ptr<FieldMap> fieldMap)
    : d_pos(-2.7, 0, 0.23, 0)
    {
      double xMax = (fieldMap->fieldLengthX() + fieldMap->outerMarginMinimum()) / 2.0;
      double yMax = (fieldMap->fieldLengthY() + fieldMap->outerMarginMinimum()) / 2.0;
      std::default_random_engine generator;
      std::uniform_real_distribution<double> fieldXDistribution(-xMax, xMax);
      std::uniform_real_distribution<double> fieldYDistribution(-yMax, yMax);
      std::uniform_real_distribution<double> thetaDistribution(-M_PI, M_PI);
      d_fieldXRng = std::bind(fieldXDistribution, generator);
      d_fieldYRng = std::bind(fieldYDistribution, generator);
      d_thetaRng = std::bind(thetaDistribution, generator);

      auto samplerFactory = std::make_shared<WheelSamplerFactory<3>>();

      auto randomState = [this]() -> ParticleFilter<3>::State
      {
        // generate an initial random state
        // TODO can we use the game mode to bias the randomness? eg before kickoff, will be on a known side, or nearer prior known locations
        return ParticleFilter<3>::State(d_fieldXRng(), d_fieldYRng(), d_thetaRng());
      };

      d_filter = std::make_shared<ParticleFilter<3>>(200, randomState, samplerFactory);

      updateState();
    }

    void predict(/*motion data*/)
    {

    }

    void update()
    {
      // TODO implement

      auto pos = d_filter->extract().first;
      double torsoHeight = AgentState::getInstance().get<BodyState>()->getTorsoHeight();
      d_pos = AgentPosition(pos[0], pos[1], torsoHeight, pos[2]);
    }

    AgentPosition position() const { return d_pos; }

  private:
    void updateState()
    {
      auto const& particleState = std::make_shared<ParticleState const>(d_filter->getParticles());

      AgentState::getInstance().set(particleState);
    }

    AgentPosition d_pos;
    std::shared_ptr<ParticleFilter<3>> d_filter;
    std::function<double()> d_fieldXRng;
    std::function<double()> d_fieldYRng;
    std::function<double()> d_thetaRng;
  };
}

#endif