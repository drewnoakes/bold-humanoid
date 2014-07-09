#include "fsmoption.hh"

using namespace bold;

FSMTransition::FSMTransition(std::string const& name)
: name(name)
{}

FSMTransition* FSMTransition::when(std::function<bool()> condition)
{
  this->conditionFactory = nullptr;
  this->condition = condition;
  return this;
}

FSMTransition* FSMTransition::when(std::function<std::function<bool()>()> conditionFactory)
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

FSMTransition* FSMTransition::notify(std::function<void()> callback)
{
  this->onFire.connect(callback);
  return this;
}

FSMTransition* FSMTransition::after(std::chrono::milliseconds time)
{
  return when([this,time] { return parentState->timeSinceStart() >= time; });
}
