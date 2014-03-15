#include "behaviourcontrol.hh"

#include "../Agent/agent.hh"
#include "../State/state.hh"
#include "../StateObject/BehaviourControlState/behaviourcontrolstate.hh"

using namespace bold;
using namespace std;

BehaviourControl::BehaviourControl(Agent& agent)
: d_agent(agent)
{
  agent.onThinkEnd.connect([this]() { updateStateObject(); });
}

void BehaviourControl::updateStateObject() const
{
  State::make<BehaviourControlState>(d_agent.getThinkCycleNumber(), d_playerRole, d_playerActivity, d_playerStatus);
}
