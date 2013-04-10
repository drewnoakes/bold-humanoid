#ifndef BOLD_FSMOPTION_HH
#define BOLD_FSMOPTION_HH

#include "../option.hh"
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

    struct Transition;
    typedef std::shared_ptr<Transition> TransitionPtr;

    struct State
    {
      State(std::string const& n, OptionPtr o, bool f = false)
      : name(n),
        final(f),
        option(o)
      {}

      /// State name
      std::string name;
      /// Whether this state is a final state
      bool final;
      /// Option to return while in this state
      OptionPtr option;
      /// Outgoing transitions
      std::vector<TransitionPtr> transitions;

      TransitionPtr newTransition()
      {
        TransitionPtr t = std::make_shared<Transition>();
        transitions.push_back(t);
        return t;
      }
    };

    struct Transition
    {
      /// Condition that needs to be fulfilled to fire
      std::function<bool()> condition;
      /// State this transition+ results in
      StatePtr nextState;
    };

  public:
    FSMOption(std::string const& id) : Option(id) {}

    virtual bool isAvailable() override;

    virtual double hasTerminated() override;

    virtual OptionPtr runPolicy() override;

    void addState(StatePtr state, bool startState = false);

    StatePtr newState(std::string const& name, OptionPtr option, bool finalState = false, bool startState = false)
    {
      auto s = std::make_shared<State>(name, option, finalState);
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
