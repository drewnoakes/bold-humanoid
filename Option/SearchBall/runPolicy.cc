#include "searchball.ih"

#include "../LookAtBall/lookatball.hh"
#include "../LookAtFeet/lookatfeet.hh"

using namespace Eigen;

vector<shared_ptr<Option>> SearchBall::runPolicy(Writer<StringBuffer>& writer)
{
  auto agentFrame = State::get<AgentFrameState>();

  static Setting<double>* turnSpeed = Config::getSetting<double>("options.circle-ball.turn-speed-a");

  double a = turnSpeed->getValue();

  d_ambulator->setTurnAngle(a);

  return {};
}

void SearchBall::setIsLeftTurn(bool leftTurn)
{
  d_isLeftTurn = leftTurn;
}

void SearchBall::reset()
{
  d_ambulator->reset();
}
