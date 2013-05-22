%{
#include <Option/FSMOption/fsmoption.hh>
%}

namespace bold
{
  struct FSMState;
  typedef std::shared_ptr<FSMState> FSMStatePtr;

  struct FSMTransition
  {
    FSMTransition(std::string const& n);
    std::string name;
    std::function<bool()> condition;
    std::function<void()> onFire;
    FSMStatePtr parentState;
    FSMStatePtr childState;
  };

  typedef std::shared_ptr<FSMTransition> FSMTransitionPtr;

  struct FSMState
  {
    FSMState(std::string const& n, OptionList o, bool f = false);
    std::string name;
    bool final;
    std::vector<FSMTransitionPtr> transitions;
    std::vector<std::shared_ptr<Option> > options;

    double startTimeSeconds;

    double secondsSinceStart();
    bool allOptionsTerminated() const;
    FSMTransitionPtr newTransition(std::string name = "");
  };

  %extend FSMState
  {
  public:
    std::vector<std::shared_ptr<Option> > getOptions() { return $self->options;  }

  };
  /** Finite State Machine
   */
  class FSMOption : public Option
  {
  public:
    FSMOption(std::string const& id);

    virtual bool isAvailable();

    virtual double hasTerminated();

    virtual OptionList runPolicy();

    void addState(FSMStatePtr state, bool startState = false);

    FSMStatePtr newState(std::string const& name,
                         std::vector<std::shared_ptr<Option> > options,
                         bool finalState = false,
                         bool startState = false);

    void addTransition(FSMTransitionPtr transition);

    std::string toDot() const;
  };
}
