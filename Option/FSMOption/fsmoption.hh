#ifndef BOLD_FSMOPTION_HH
#define BOLD_FSMOPTION_HH

#include "../option.hh"
#include "../../Clock/clock.hh"
#include <string>
#include <vector>

namespace bold
{
  /** Finite State Machine
   */
  class FSMOption : public Option
  {
  public:

    struct State;
    typedef std::shared_ptr<State> StatePtr;

    struct Transition
    {
      /// Condition that needs to be fulfilled to fire
      std::function<bool()> condition;

      /// Function to be called when this transition is fired
      std::function<void()> onFire;

      /// State this transition goes from
      StatePtr parentState;
      /// State this transition results in
      StatePtr childState;
    };

    typedef std::shared_ptr<Transition> TransitionPtr;

    struct State : public std::enable_shared_from_this<State>
    {
      State(std::string const& n, OptionList o, bool f = false)
	: name(n),
	  final(f),
	  options(o)
      {}

      /// State name
      std::string name;
      /// Whether this state is a final state
      bool final;
      /// Options to return while in this state
      OptionList options;
      /// Outgoing transitions
      std::vector<TransitionPtr> transitions;

      double startTime;

      TransitionPtr newTransition()
      {
        TransitionPtr t = std::make_shared<Transition>();
        t->parentState = shared_from_this();
        transitions.push_back(t);
        return t;
      }
    };

  public:
    FSMOption(std::string const& id) : Option(id) {}

    virtual bool isAvailable() override;

    virtual double hasTerminated() override;

    virtual OptionList runPolicy() override;

    void addState(StatePtr state, bool startState = false);

    StatePtr newState(std::string const& name,
		      OptionList options,
		      bool finalState = false,
		      bool startState = false)
    {
      auto s = std::make_shared<State>(name, options, finalState);
      addState(s, startState);
      return s;
    }

    void addTransition(TransitionPtr transition);

  private:
    std::vector<StatePtr> d_states;
    std::vector<TransitionPtr> d_transitions;
    StatePtr d_startState;
    StatePtr d_curState;
  };



  inline bool FSMOption::isAvailable()
  {
    return true;
  }

  inline double FSMOption::hasTerminated()
  {
    return d_curState && d_curState->final ? 1.0 : 0.0;
  }

  inline void FSMOption::addState(FSMOption::StatePtr state, bool startState)
  {
    d_states.push_back(state);
    if (startState)
      d_startState = state;
  }

  inline void FSMOption::addTransition(FSMOption::TransitionPtr transition)
  {
    d_transitions.push_back(transition);
  }
}

#endif
