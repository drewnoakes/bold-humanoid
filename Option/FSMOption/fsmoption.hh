#pragma once

#include "../option.hh"
#include "../../Clock/clock.hh"
#include "../../util/assert.hh"
#include "../../util/websocketbuffer.hh"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <sigc++/signal.h>

namespace bold
{
  struct FSMState;
  template<typename> class Setting;
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
    FSMTransition* after(std::chrono::milliseconds time);
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

    std::chrono::duration<double> timeSinceStart() const;
    double secondsSinceStart() const;

    bool allOptionsTerminated() const;

    std::shared_ptr<FSMTransition> newTransition(std::string name = "");

    std::shared_ptr<FSMTransition> transitionTo(std::shared_ptr<FSMState>& targetState, std::string name = "");
  };

  /** Finite State Machine
   */
  class FSMOption : public Option
  {
  public:
    FSMOption(std::shared_ptr<Voice> voice, std::string const& id);

    virtual double hasTerminated() override;

    virtual void reset() override;

    virtual std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    void addState(std::shared_ptr<FSMState> state, bool startState = false);

    std::shared_ptr<FSMState> getState(std::string name) const;
    std::shared_ptr<FSMState const> getCurrentState() const { return d_curState; }

    std::shared_ptr<FSMState> newState(std::string const& name,
                         std::vector<std::shared_ptr<Option>> options,
                         bool finalState = false,
                         bool startState = false)
    {
      auto s = std::make_shared<FSMState>(name, options, finalState);
      addState(s, startState);
      return s;
    }

    std::shared_ptr<FSMState> const& getStartState() const { return d_startState; }

    std::shared_ptr<FSMTransition> wildcardTransitionTo(std::shared_ptr<FSMState> targetState, std::string name = "");

    std::string toDot() const;

    void toJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const { toJsonInternal(writer); }
    void toJson(rapidjson::Writer<WebSocketBuffer>& writer) const { toJsonInternal(writer); }

  private:
    template<typename TBuffer>
    void toJsonInternal(rapidjson::Writer<TBuffer>& writer) const;

    void setCurrentState(std::shared_ptr<FSMState> state);

    std::vector<std::shared_ptr<FSMState>> d_states;
    std::vector<std::shared_ptr<FSMTransition>> d_wildcardTransitions;
    std::shared_ptr<FSMState> d_startState;
    std::shared_ptr<FSMState> d_curState;
    std::shared_ptr<Voice> d_voice;
    Setting<bool>* d_paused;
  };

  template<typename TBuffer>
  void FSMOption::toJsonInternal(rapidjson::Writer<TBuffer>& writer) const
  {
    /*
     {
       "name": "win",
       "start": "startup",
       "states": [
         { "id": "startup" }
       ],
       "transitions": [
         { "from": "startup", "to": "ready", "label": "init" }
       ],
       "wildcardTransitions": [
         { "to": "ready", "label": "init" }
       ]
     }
    */
    writer.StartObject();
    {
      writer.String("name");
      writer.String(getId().c_str());
      writer.String("start");
      writer.String(d_startState->name.c_str());

      writer.String("states");
      writer.StartArray();
      {
        for (auto const& state : d_states)
        {
          writer.StartObject();
          {
            writer.String("id");
            writer.String(state->name.c_str());
          }
          writer.EndObject();
        }
      }
      writer.EndArray();

      writer.String("transitions");
      writer.StartArray();
      {
        for (auto const& state : d_states)
        {
          for (auto const& transition : state->transitions)
          {
            writer.StartObject();
            {
              writer.String("id");
              writer.String(transition->name.c_str());
              writer.String("from");
              writer.String(state->name.c_str());
              writer.String("to");
              writer.String(transition->childState->name.c_str());
            }
            writer.EndObject();
          }
        }
      }
      writer.EndArray();

      writer.String("wildcardTransitions");
      writer.StartArray();
      {
        for (auto const& transition : d_wildcardTransitions)
        {
          writer.StartObject();
          {
            writer.String("id");
            writer.String(transition->name.c_str());
            writer.String("to");
            writer.String(transition->childState->name.c_str());
          }
          writer.EndObject();
        }
      }
      writer.EndArray();
    }
    writer.EndObject();
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
      ASSERT(!d_startState);
      d_startState = state;
    }
  }

  inline std::shared_ptr<FSMTransition> FSMOption::wildcardTransitionTo(std::shared_ptr<FSMState> targetState, std::string name)
  {
    std::shared_ptr<FSMTransition> t = std::make_shared<FSMTransition>(name);
    t->childState = targetState;
    d_wildcardTransitions.push_back(t);
    return t;
  }
}
