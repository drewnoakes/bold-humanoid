#include "localiser.ih"

void Localiser::update()
{
  auto const& agentFrame = AgentState::get<AgentFrameState>();

  d_filter->predict([this](Vector3d state) -> Vector3d
  {
    return Vector3d(
      state[0] + d_positionError(),
      state[1] + d_positionError(),
      state[2] + d_angleError()
    );
  });

  d_filter->update([&agentFrame,this](Vector3d state) -> double
  {
    AgentPosition pos(state[0], state[1], state[2]);

    Affine3d agentWorld3d(pos.agentWorldTransform());
    Affine3d worldAgent3d(pos.worldAgentTransform());

    // TODO avoid 2d->3d->2d conversion here by creating a worldToAgent2d transform

    // IDEA further positions have more error, so should carry lesser reward? result: bias slightly towards closer matches

    double scoreSum = 0;

    //
    // Score observed lines
    //

    if (d_useLines)
    {
      for (LineSegment3d const& observed : agentFrame->getObservedLineSegments())
      {
        LineSegment2d observed2d(observed.to<2>());

        double bestScore = 0;

        for (LineSegment3d const& candidate : d_fieldMap->getFieldLines())
        {
          LineSegment2d candidateAgent = LineSegment3d(agentWorld3d * candidate.p1(), agentWorld3d * candidate.p2()).to<2>();

          // very naive scoring system for now...

          double distance1 = (observed2d.p1() - Math::linePointClosestToPoint(candidateAgent, observed2d.p1())).norm();
          double distance2 = (observed2d.p2() - Math::linePointClosestToPoint(candidateAgent, observed2d.p2())).norm();

          double score = d_rewardFalloff / (distance1 + distance2 + d_rewardFalloff);

          double angle = observed2d.smallestAngleBetween(candidateAgent);
          score *= angle/(M_PI/2);

          if (score > bestScore)
            bestScore = score;
        }

        // Now evaluate it as a circle line
        Vector2d wp1 = (worldAgent3d * observed.p1()).head<2>();
        Vector2d wp2 = (worldAgent3d * observed.p2()).head<2>();
        double circDist1 = wp1.norm() - d_fieldMap->circleRadius();
        double circDist2 = wp2.norm() - d_fieldMap->circleRadius();
        double circScore = d_rewardFalloff / (circDist1 + circDist2 + d_rewardFalloff);

        if (circScore > bestScore)
        {
          // Looks like it might be a circle line -- however it's not likely if it bisects much of the circle
          if (Math::smallestAngleBetween(wp1, wp2) < M_PI/6)
            bestScore = circScore;
        }

        scoreSum += bestScore;
      }
    }

    //
    // Score observed goal posts
    //

    if (agentFrame->getGoalObservations().size() >= d_minGoalsNeeded)
    {
      for (Vector3d const& observed : agentFrame->getGoalObservations())
      {
        Vector2d observed2d(observed.head<2>());
        double bestScore = 0;

        for (Vector3d const& candidate : d_fieldMap->getGoalPostPositions())
        {
          Vector3d candidate3d(candidate.x(), candidate.y(), 0);
          Vector3d candidateAgent3d(agentWorld3d * candidate3d);
          Vector2d candidateAgent2d(candidateAgent3d.x(), candidateAgent3d.y());

          // very naive scoring system for now...

          double distance = (candidateAgent2d - observed2d).norm();

          double score = 1 / distance;

          if (score > bestScore)
            bestScore = score;
        }

        scoreSum += bestScore;
      }
    }

    assert(scoreSum >= 0);

    return scoreSum;
  });

  auto state = d_filter->extract();

  d_pos = AgentPosition(state[0], state[1], state[2]);

  updateSmoothedPos();

  // Copy particles into their state object
  updateStateObject();
}
