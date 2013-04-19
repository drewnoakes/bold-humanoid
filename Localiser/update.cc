#include "localiser.ih"

void Localiser::update()
{
  // TODO d_filter->predict();

  auto pos = d_filter->extract().first;
  double torsoHeight = AgentState::get<BodyState>()->getTorsoHeight();
  d_pos = AgentPosition(pos[0], pos[1], torsoHeight, pos[2]);

  updateStateObject();
}