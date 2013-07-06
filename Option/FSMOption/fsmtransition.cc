#include "fsmoption.hh"

using namespace bold;

FSMTransition::FSMTransition(std::string const& name)
: name(name)
{}

FSMTransition* FSMTransition::when(std::function<bool()> condition)
{
  this->condition = condition;
  return this;
}

FSMTransition* FSMTransition::notify(std::function<void()> callback)
{
  this->onFire = callback;
  return this;
}

FSMTransition* FSMTransition::whenTerminated()
{
  this->condition = [this]() { return parentState->allOptionsTerminated(); };
  return this;
}
