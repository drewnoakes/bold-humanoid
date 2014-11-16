#include "fsmoption.hh"

#include <algorithm>

using namespace bold;
using namespace std;

FSMState::FSMState(string const& name, vector<shared_ptr<Option>> options, bool isFinal)
: name(name),
  isFinal(isFinal),
  options(options),
  onEnter(),
  startTimestamp()
{}

void FSMState::start()
{
  startTimestamp = Clock::getTimestamp();

  // For transitions with condition factories, invoke them to clear out any accumulated state
  for (shared_ptr<FSMTransition> const& transition : transitions)
  {
    if (transition->conditionFactory)
      transition->condition = transition->conditionFactory();
  }

  onEnter();
}

std::chrono::duration<double> FSMState::timeSinceStart() const
{
  return std::chrono::duration<double>(Clock::getSecondsSince(startTimestamp));
}

double FSMState::secondsSinceStart() const
{
  return Clock::getSecondsSince(startTimestamp);
}

bool FSMState::allOptionsTerminated() const
{
  // NOTE this isn't recursive, so if a state's child returns a sub-option that hasn't terminated, the parent must currently handle this
  return all_of(options.begin(), options.end(), [](shared_ptr<Option> const& o) { return o->hasTerminated(); });
}

shared_ptr<FSMTransition> FSMState::transitionTo(shared_ptr<FSMState>& targetState, string name)
{
  shared_ptr<FSMTransition> t = make_shared<FSMTransition>(name);
  t->parentState = shared_from_this();
  t->childState = targetState;
  transitions.push_back(t);
  return t;
}
