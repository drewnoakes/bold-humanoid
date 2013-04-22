#include "localiser.ih"

void Localiser::update()
{
  auto const& agentFrame = AgentState::get<AgentFrameState>();
  double torsoHeight = AgentState::get<BodyState>()->getTorsoHeight();

  d_filter->predict([this](Vector3d state) -> Vector3d
  {
    return Vector3d(
      state[0] + d_positionError(),
      state[1] + d_positionError(),
      state[2] + d_angleError()
    );
  });

  d_filter->update([&torsoHeight,&agentFrame,this](Vector3d state) -> double
  {
    AgentPosition pos(state[0], state[1], torsoHeight, state[2]);
    Affine3d worldToAgent(pos.worldToAgentTransform());

    double scoreSum = 0;

    for (LineSegment3d const& observed : agentFrame->getObservedLineSegments())
    {
      LineSegment2d const& observed2d = observed.to<2>();
      double bestScore = 0;
      for (LineSegment2d const& worldLine : d_fieldMap->getFieldLines())
      {
        Vector3d p1(worldLine.p1().x(), worldLine.p1().y(), torsoHeight);
        Vector3d p2(worldLine.p2().x(), worldLine.p2().y(), torsoHeight);
        LineSegment3d candidate3d(worldToAgent * p1, worldToAgent * p2);
        LineSegment2d candidate2d = candidate3d.to<2>();

        // very naive scoring system for now...

        double distance1 = (observed2d.p1() - Math::linePointClosestToPoint(candidate2d, observed2d.p1())).norm();
        double distance2 = (observed2d.p2() - Math::linePointClosestToPoint(candidate2d, observed2d.p2())).norm();

        double score = 1 / (distance1 + distance2);

        if (score > bestScore)
          bestScore = score;
      }

      scoreSum += bestScore;
    }

    return scoreSum;
  });

  auto state = d_filter->extract();

  d_pos = AgentPosition(state[0], state[1], torsoHeight, state[2]);

  updateStateObject();
}
