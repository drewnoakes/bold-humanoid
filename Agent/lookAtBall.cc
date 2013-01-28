#include "agent.ih"

void Agent::lookAtBall()
{
  auto ballObs = find_if(d_observations.begin(), d_observations.end(),
			 [](Observation const& obs) { return obs.type == O_BALL; });

  bool ballSeen = ballObs != d_observations.end();
  if (!ballSeen)
  {
    cout << "Look at ball, but no ball seen" << endl;
    return;
  }

  Vector2f foundBallAtPx = ballObs->pos;

  lookAt(foundBallAtPx);
}
