#include "localiser.ih"

void Localiser::update()
{
  auto const& agentFrame = State::get<AgentFrameState>();

  predict();

  JointObservationModel<3> jointModel;

  if (agentFrame->getGoalObservations().size() >= static_cast<uint>(d_minGoalsNeeded->getValue()))
  {
    auto goalPostModel = [&](Vector3d const& state) {
      AgentPosition pos(state[0], state[1], state[2]);
      Affine3d agentWorld3d(pos.agentWorldTransform());
      Affine3d worldAgent3d(pos.worldAgentTransform());

      double scoreProd = 1.0;
      unsigned cnt = 0;
      for (Vector3d const& observed : agentFrame->getGoalObservations())
      {
        ++cnt;
        if (cnt > 2)
          break;

        Vector2d observed2d(observed.head<2>());
        double bestScore = 0;

        for (Vector3d const& candidate : FieldMap::getGoalPostPositions())
        {
          Vector3d candidate3d(candidate.x(), candidate.y(), 0);
          Vector3d candidateAgent3d(agentWorld3d * candidate3d);
          Vector2d candidateAgent2d(candidateAgent3d.x(), candidateAgent3d.y());

          // very naive scoring system for now...

          double distance = (candidateAgent2d - observed2d).norm();

          double score = exp(-distance*distance);

          if (score > bestScore)
            bestScore = score;
        }
        scoreProd *= bestScore;
      }
      return scoreProd;
    };

    jointModel.addModel(goalPostModel);
  }

  if (d_useLines->getValue() &&  agentFrame->getObservedLineSegments().size() > 0)
  {
    log::verbose("Localiser::update") << "Using line model";
    auto lineModel = [&](Vector3d const& state) {
      AgentPosition pos(state[0], state[1], state[2]);

      Affine3d agentWorld3d(pos.agentWorldTransform());
      Affine3d worldAgent3d(pos.worldAgentTransform());

      // TODO avoid 2d->3d->2d conversion here by creating a worldToAgent2d transform

      // IDEA further positions have more error, so should carry lesser reward? result: bias slightly towards closer matches

      double scoreSum = 0;

      //
      // Score observed lines
      //

      for (LineSegment3d const& observed : agentFrame->getObservedLineSegments())
      {
        LineSegment2d observed2d(observed.to<2>());

        double bestScore = 0;

        for (LineSegment3d const& candidate : FieldMap::getFieldLines())
        {
          LineSegment2d candidateAgent = LineSegment3d(agentWorld3d * candidate.p1(), agentWorld3d * candidate.p2()).to<2>();

          // very naive scoring system for now...

          double distance1 = (observed2d.p1() - Math::linePointClosestToPoint(candidateAgent, observed2d.p1())).norm();
          double distance2 = (observed2d.p2() - Math::linePointClosestToPoint(candidateAgent, observed2d.p2())).norm();
          double distance = distance1 + distance2;
          double score = exp(-distance * distance);

          if (score > bestScore)
            bestScore = score;
        }
        scoreSum += bestScore;
      }
      return scoreSum / agentFrame->getObservedLineSegments().size();
    };
    jointModel.addModel(lineModel);
  }

  d_filter->update(jointModel);
  auto weights = d_filter->getWeights();
  d_preNormWeightSum = weights.sum();
  d_filter->normalize();

  auto stateWeight = d_filter->extract();

  d_pos = AgentPosition(stateWeight.first.x(), stateWeight.first.y(), stateWeight.first.z());

  updateSmoothedPos();

  // Copy particles into their state object
  updateStateObject();
}
