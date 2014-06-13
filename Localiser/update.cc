#include "localiser.ih"

void Localiser::update()
{
  auto const& agentFrame = State::get<AgentFrameState>();

  predict();

  JointObservationModel<4> jointModel;

  if (agentFrame->getGoalObservations().size() >= static_cast<uint>(d_minGoalsNeeded->getValue()))
  {
    auto goalPostModel = [&](Vector4d const& state) {
      AgentPosition pos(state);
      Affine3d agentWorld3d(pos.agentWorldTransform());

      double scoreProd = 1.0;
      unsigned cnt = 0;
      for (Vector3d const& observed : agentFrame->getGoalObservations())
      {
        ++cnt;
        if (cnt > 2)
          break;

        Vector2d observed2d{observed.head<2>()};
        double bestScore = 0;

        for (Vector3d const& candidate : FieldMap::getGoalPostPositions())
        {
          Vector3d candidate3d{candidate.x(), candidate.y(), 0};
          Vector3d candidateAgent3d{agentWorld3d * candidate3d};
          Vector2d candidateAgent2d{candidateAgent3d.head<2>()};

          // very naive scoring system for now...

          double distance = (candidateAgent2d - observed2d).norm();

          double score = exp(-distance*distance / 10);

          if (score > bestScore)
            bestScore = score;
        }
        scoreProd *= bestScore;
      }
      return scoreProd;
    };

    jointModel.addModel(goalPostModel);
  }

  if (d_useLines->getValue() && agentFrame->getObservedLineJunctions().size() > 0)
  {
    log::verbose("Localiser::update") << "Using line junction model";
    auto lineModel = [&](Vector4d const& state) {
      AgentPosition pos(state);

      Affine3d agentWorld3d(pos.agentWorldTransform());

      double scoreProd = 1.0;
      for (auto const& observedLineJunction : agentFrame->getObservedLineJunctions())
      {
        Vector2d observedLineJunction2d{observedLineJunction.position};
        double bestScore = 0;
        for (auto const& candidate : FieldMap::getFieldLineJunctions())
        {
          if (candidate.type != observedLineJunction.type)
            continue;

          Vector3d candidate3d{candidate.position.x(), candidate.position.y(), 0};
          Vector3d candidateAgent3d{agentWorld3d * candidate3d};
          Vector2d candidateAgent2d{candidateAgent3d.head<2>()};

          // very naive scoring system for now...

          double distance = (candidateAgent2d - observedLineJunction2d).norm();

          double score = exp(-distance*distance / 10);

          if (score > bestScore)
            bestScore = score;
        }
        scoreProd *= bestScore;
      }

      return scoreProd;
    };
    jointModel.addModel(lineModel);
  }

  d_filter->update(jointModel);
  if (d_filterType == FilterType::Particle)
  {
    auto filter = static_pointer_cast<ParticleFilterUsed>(d_filter);

    auto weights = filter->getWeights();
    d_preNormWeightSum = weights.sum();
    filter->normalize();
  }

  auto stateWeight = d_filter->extract();

  d_pos = AgentPosition(stateWeight.first);
  d_uncertainty = stateWeight.second;

  updateSmoothedPos();

  // Copy particles into their state object
  updateStateObject();
}
