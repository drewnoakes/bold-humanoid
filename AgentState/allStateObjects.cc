#include "agentstate.hh"

using namespace bold;
using namespace std;

vector<shared_ptr<StateObject>> AgentState::allStateObjects() const
{
  vector<shared_ptr<StateObject>> stateObjects;
  for (auto const& pair : d_stateByTypeId) {
    stateObjects.push_back(pair.second);
  }
  return stateObjects;
}