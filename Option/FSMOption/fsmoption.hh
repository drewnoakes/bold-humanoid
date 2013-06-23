#pragma once

#include "../option.hh"
#include "../../Clock/clock.hh"

#include <string>
#include <vector>
#include <algorithm>
#include <memory>

namespace bold
{
  struct FSMState;
  typedef std::shared_ptr<FSMState> FSMStatePtr;

  struct FSMTransition
  {
    FSMTransition(std::string const& n)
      : name(n)
    {}

    // Transition name
    std::string name;

    /// Condition that needs to be fulfilled to fire
    std::function<bool()> condition;

    /// Function to be called when this transition is fired
    std::function<void()> onFire;

    /// State this transition goes from
    FSMStatePtr parentState;
    /// State this transition results in
    FSMStatePtr childState;

    FSMTransition* when(std::function<bool()> condition)
    {
      this->condition = condition;
      return this;
    }

    FSMTransition* notify(std::function<void()> callback)
    {
      this->onFire = callback;
      return this;
    }
  };

  typedef std::shared_ptr<FSMTransition> FSMTransitionPtr;

  struct FSMState : public std::enable_shared_from_this<FSMState>
  {
    FSMState(std::string const& n, std::vector<std::shared_ptr<Option>> o, bool f = false)
      : name(n),
        final(f),
        options(o)
    {}

    /// State name
    std::string name;
    /// Whether this state is a final state
    bool final;
    /// Options to return while in this state
    std::vector<std::shared_ptr<Option>> options;
    /// Outgoing transitions
    std::vector<FSMTransitionPtr> transitions;

    std::function<void()> onEnter;

    double startTimeSeconds;

    double secondsSinceStart()
    {
      return Clock::getSeconds() - startTimeSeconds;
    }

    bool allOptionsTerminated() const
    {
      return std::all_of(options.begin(), options.end(), [](std::shared_ptr<Option> o) { return o->hasTerminated(); });
    }

    FSMTransitionPtr newTransition(std::string name = "")
    {
      FSMTransitionPtr t = std::make_shared<FSMTransition>(name);
      t->parentState = shared_from_this();
      transitions.push_back(t);
      return t;
    }

    FSMTransitionPtr transitionTo(FSMStatePtr targetState)
    {
      FSMTransitionPtr t = std::make_shared<FSMTransition>("");
      t->parentState = shared_from_this();
      t->childState = targetState;
      transitions.push_back(t);
      return t;
    }
  };

  /** Finite State Machine
   */
  class FSMOption : public Option
  {
  public:
    FSMOption(std::string const& id) : Option(id) {}

    virtual bool isAvailable() override;

    virtual double hasTerminated() override;

    virtual std::vector<std::shared_ptr<Option>> runPolicy() override;

    void addState(FSMStatePtr state, bool startState = false);

    FSMStatePtr newState(std::string const& name,
                         std::vector<std::shared_ptr<Option>> options,
                         bool finalState = false,
                         bool startState = false)
    {
      auto s = std::make_shared<FSMState>(name, options, finalState);
      addState(s, startState);
      return s;
    }

    FSMTransitionPtr wildcardTransitionTo(FSMStatePtr targetState);

    std::string toDot() const;

  private:
    std::vector<FSMStatePtr> d_states;
    std::vector<FSMTransitionPtr> d_wildcardTransitions;
    FSMStatePtr d_startState;
    FSMStatePtr d_curState;
  };



  inline bool FSMOption::isAvailable()
  {
    return true;
  }

  inline double FSMOption::hasTerminated()
  {
    return d_curState && d_curState->final ? 1.0 : 0.0;
  }

  inline void FSMOption::addState(FSMStatePtr state, bool startState)
  {
    d_states.push_back(state);
    if (startState)
      d_startState = state;
  }

  inline FSMTransitionPtr FSMOption::wildcardTransitionTo(FSMStatePtr targetState)
  {
    FSMTransitionPtr t = std::make_shared<FSMTransition>("");
    t->childState = targetState;
    d_wildcardTransitions.push_back(t);
    return t;
  }
}
