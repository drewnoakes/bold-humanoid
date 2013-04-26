#include "fsmoption.ih"

string FSMOption::toDot() const
{
  ostringstream out;
  out << "digraph "  << getID() << "{" << endl;
  list<StatePtr> stateQueue;
  stateQueue.push_back(d_startState);

  list<StatePtr> visitedStates;

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
        out << state->name << " -> " << transition->childState->name << endl;
        
        stateQueue.push_back(transition->childState);
      }
      visitedStates.push_back(state);
    }
  }

  out << "}" << endl;

  return out.str();
}
