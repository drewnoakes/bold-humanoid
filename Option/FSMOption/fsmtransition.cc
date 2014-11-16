#include "fsmoption.hh"

using namespace bold;
using namespace std;

FSMTransition::FSMTransition(string const& name)
: name(name)
{}

FSMTransition* FSMTransition::when(function<bool()> condition)
{
  this->conditionFactory = nullptr;
  this->condition = condition;
  return this;
}

FSMTransition* FSMTransition::when(function<function<bool()>()> conditionFactory)
{
  this->conditionFactory = conditionFactory;
  this->condition = nullptr;
  return this;
}

FSMTransition* FSMTransition::whenTerminated()
{
  this->condition = [this] { return parentState->allOptionsTerminated(); };
  return this;
}

FSMTransition* FSMTransition::notify(function<void()> callback)
{
  this->onFire.connect(callback);
  return this;
}

FSMTransition* FSMTransition::after(chrono::milliseconds time)
{
  return when([this,time] { return parentState->timeSinceStart() >= time; });
}
