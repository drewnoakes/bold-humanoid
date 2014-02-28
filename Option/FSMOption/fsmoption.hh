#pragma once

#include "../option.hh"
#include "../../Clock/clock.hh"

#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <sigc++/signal.h>

namespace bold
{
  struct FSMState;
  class Voice;

  struct FSMTransition
  {
    FSMTransition(std::string const& name);

    std::string name;

    /// Condition that needs to be fulfilled to fire
    std::function<bool()> condition;

    /// A function that creates a new condition function each time the source
    /// FSMState is entered. This prevents conditions from carrying state
    /// between usages.
    std::function<std::function<bool()>()> conditionFactory;

    /// Function to be called when this transition is fired
    sigc::signal<void> onFire;

    /// State this transition goes from
    std::shared_ptr<FSMState> parentState;
    /// State this transition results in
    std::shared_ptr<FSMState> childState;

    FSMTransition* when(std::function<bool()> condition);
    FSMTransition* when(std::function<std::function<bool()>()> conditionFactory);
    FSMTransition* whenTerminated();
    FSMTransition* notify(std::function<void()> callback);
  };

  struct FSMState : public std::enable_shared_from_this<FSMState>
  {
    FSMState(std::string const& name, std::vector<std::shared_ptr<Option>> options, bool isFinal = false);

    /// State name
    std::string name;
    /// Whether this state is a final state
    bool isFinal;
    /// Options to return while in this state
    std::vector<std::shared_ptr<Option>> options;
    /// Outgoing transitions
    std::vector<std::shared_ptr<FSMTransition>> transitions;

    sigc::signal<void> onEnter;

    Clock::Timestamp startTimestamp;

    void start();

    double secondsSinceStart() const;

    bool allOptionsTerminated() const;

    std::shared_ptr<FSMTransition> newTransition(std::string name = "");

    std::shared_ptr<FSMTransition> transitionTo(std::shared_ptr<FSMState> targetState, std::string name = "");
  };

  /** Finite State Machine
   */
  class FSMOption : public Option
  {
  public:
    FSMOption(std::shared_ptr<Voice> voice, std::string const& id)
    : Option(id),
      d_voice(voice)
    {}

    virtual bool isAvailable() override;

    virtual double hasTerminated() override;

    virtual std::vector<std::shared_ptr<Option>> runPolicy() override;

    void addState(std::shared_ptr<FSMState> state, bool startState = false);

    std::shared_ptr<FSMState> newState(std::string const& name,
                         std::vector<std::shared_ptr<Option>> options,
                         bool finalState = false,
                         bool startState = false)
    {
      auto s = std::make_shared<FSMState>(name, options, finalState);
      addState(s, startState);
      return s;
    }

    std::shared_ptr<FSMTransition> wildcardTransitionTo(std::shared_ptr<FSMState> targetState);

    std::string toDot() const;

    void toJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const;

  private:
    std::vector<std::shared_ptr<FSMState>> d_states;
    std::vector<std::shared_ptr<FSMTransition>> d_wildcardTransitions;
    std::shared_ptr<FSMState> d_startState;
    std::shared_ptr<FSMState> d_curState;
    std::shared_ptr<Voice> d_voice;
  };

  inline bool FSMOption::isAvailable()
  {
    return true;
  }

  inline double FSMOption::hasTerminated()
  {
    return d_curState && d_curState->isFinal ? 1.0 : 0.0;
  }

  inline void FSMOption::addState(std::shared_ptr<FSMState> state, bool startState)
  {
    d_states.push_back(state);
    if (startState)
    {
      assert(!d_startState);
      d_startState = state;
    }
  }

  inline std::shared_ptr<FSMTransition> FSMOption::wildcardTransitionTo(std::shared_ptr<FSMState> targetState)
  {
    std::shared_ptr<FSMTransition> t = std::make_shared<FSMTransition>("");
    t->childState = targetState;
    d_wildcardTransitions.push_back(t);
    return t;
  }
}
