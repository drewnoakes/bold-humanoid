#include "agent.ih"

void Agent::lookForBall()
{
  Robot::Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);

  auto ballObs = find_if(d_observations.begin(), d_observations.end(),
			 [](Observation const& obs) { return obs.type == O_BALL; });

  bool ballSeen = ballObs != d_observations.end();
  //if (ballSeen)
  //  d_ballSeenCnt++;
  
  if (d_ballSeenCnt >= 10)
  {
    d_state = S_APPROACH_BALL;
    return;
  }

  // Havent seen the ball enough
  if (ballSeen)
  {
    // Look at ball
    lookAtBall();
  }
  else
  {
    // Oscillate
    
  }

}
