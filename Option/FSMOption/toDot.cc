#include "fsmoption.hh"

#include <algorithm>
#include <sstream>

#include "../../util/assert.hh"

using namespace bold;
using namespace std;

string FSMOption::toDot() const
{
  ostringstream out;
  out << "digraph "  << getId() << "{" << endl;

  ASSERT(d_startState && "FSM must have a starting state");

  list<shared_ptr<FSMState>> stateQueue{d_startState};
  list<shared_ptr<FSMState>> visitedStates;

  while (!stateQueue.empty())
  {
    auto state = stateQueue.front();
    stateQueue.pop_front();
    if (find(visitedStates.begin(), visitedStates.end(), state) == visitedStates.end())
    {
      out << state->name << "[";
      if (state == d_startState)
      {
        out << "shape=doublecircle";
      }
      out << "]" << endl;

      for (auto transition : state->transitions)
      {
        out << state->name << " -> " << transition->childState->name;
        if (transition->name != "")
          out << " [label=" << transition->name << "]";
        out << endl;

        stateQueue.push_back(transition->childState);
      }
      visitedStates.push_back(state);
    }
  }

  out << "}" << endl;

  return out.str();
}
