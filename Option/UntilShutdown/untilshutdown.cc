#include "untilshutdown.hh"

#include "../../Agent/agent.hh"
#include "../../StateObject/TeamState/teamstate.hh"
#include "../../BehaviourControl/behaviourcontrol.hh"

using namespace bold;
using namespace std;
using namespace rapidjson;

UntilShutdown::UntilShutdown(string id, Agent* agent, shared_ptr<Option> beforeShutdown, shared_ptr<Option> afterShutdown)
: Option(id, "UntilShutdown"),
  d_agent(agent),
  d_beforeShutdown(beforeShutdown),
  d_afterShutdown(afterShutdown)
{}

vector<shared_ptr<Option>> UntilShutdown::runPolicy(Writer<StringBuffer>& writer)
{
  if (d_agent->isStopRequested())
  {
    d_agent->getBehaviourControl()->setPlayerActivity(PlayerActivity::Waiting);
    d_agent->getBehaviourControl()->setPlayerStatus(PlayerStatus::Inactive);
    return { d_afterShutdown };
  }
  else
  {
    return { d_beforeShutdown };
  }
}
