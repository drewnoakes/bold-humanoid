import bold
import time

class FSMTransition:
    
    """Finite State Machine transition

    Represents a transition between states in an FSM.

    Attributes:
        name: A string giving a name to this transition; only used in
          output.
        condition: A function, type: foo() -> bool, that determines
          when this transition is fired
        onFire: A function, type: foo() -> void, that is called when
          the condition is true and this transition is fired; can be
          None.
        parentState: The FSMState that this transition is from
        childState: The FSMState that this transition is to
    """
    
    def __init__(self, name, parentState = None):
        self.name = name
        self.condition = None
        self.onFire = None
        self.parentState = parentState
        self.childState = None

    def when(self, condition):
        self.condition = condition
        return self

    def notify(self, callback):
        self.onFire = callback
        return self

class FSMState:

    """Finite State Machine state

    Represents a state in an FSM.

    Attributes:
        name: A string giving the name of this state; only used in
          output.
        options: A list of options that are run when this state is
          active.
        final: Boolean flag, denoting whether this is a final
          state. The FSMOption containing this state will report
          having terminated if this is true and this state is active.
        transitions: A list of FSMTransition objects that determine
          the possible transitions out of this state.
        startTimeSeconds: The time, measured with time.time() in seconds,
          when this action became active.
    """

    def __init__(self, name, options = [], final = False):
        self.name = name
        self.options = options
        self.final = final
        self.transitions = []
        self.startTimeSeconds = 0
        self.onEnter = None

    def secondsSinceStart(self):
        """Return the difference between now and start time. """
        return time.time() - self.startTimeSeconds

    def allOptionsTerminated(self):
        """Return whether all options in this state report
        hasTerminated() != 0."""
        return all(o.hasTerminated() for o in self.options)

    def newTransition(self, name = "", childState = None):
        """Add a new transition from this state, and return it"""
        t = FSMTransition(name, self)
        t.childState = childState
        self.transitions.append(t)
        return t
    
    def transitionTo(self, targetState):
        t = FSMTransition("")
        t.parentState = self
        t.childState = targetState
        self.transitions.append(t)
        return t

        
class FSMOption(bold.Option):
    
    """ Finite State Machine Option

    A Finite State Machine (FSM) is a directed graph, where the nodes
    are states (represented by FMState objects) and the edges define
    transitions between these states (represented by FSMTransition
    objects). When run, the FSM performs the following steps:
    
    * If no state is selected, select the start state
    * Loop through the transitions from this state and check their
      conditions; fire the first one that returns True.
    * Repeat the previous step, until no transitions fire anymore.
    * Return the list of options associated with the final selected
      state.
    
    Attributes:
      states: List of FSMState objects
      transitions: List of FSMTransition objects
      startState: The FSMState object to start the machine in
      curState: The currently selected FSMState
    """

    def __init__(self, id):
        bold.Option.__init__(self, id)
        self.states = []
        self.transitions = []

        self.startState = None
        self.curState = None

    def isAvailable(self):
        """ Always return True.

        Overrides Option.isAvailable.
        """
        return True

    def hasTerminated(self):
        """ Return 1.0 if current state is final, 0.0 otherwise.

        Overrides bold.Option.hasTerminated.
        """
        if not self.curState is None and self.curState.final:
            return 1.0
        else:
            return 0.0

    def addState(self, state, startState = False):
        """Add a state to this FSM
        
        Args:
          state: FSMState object to add.
          startState: Boolean flag denoting whether this is the start
            state. If true, this replaces any previously set start
            state.
        """
        self.states.append(state)
        if startState:
            self.startState = state

    def newState(self, name = "", options = [], startState = False, finalState = False):
        s = FSMState(name, options, finalState)
        self.addState(s, startState)
        return s

    def addTransition(self, transition):
        """Add a transition to this FSM
        
        Args:
          transition: FSMTransition object to add.
        """
        self.transitions.append(transition)

    def runPolicy(self):
        if self.curState is None:
            self.curState = self.startState
            if self.curState.onEnter is not None:
                self.curState.onEnter()

        MAX_LOOP_COUNT = 20

        loopCount = 0
        transitionMade = False
        while True:
            transitionMade = False
            for t in self.curState.transitions:
                if (t.condition()):
                    print("[FSMOption::runPolicy] (" + self.getID() + ") transitioning from '" +
                          self.curState.name + "' to '" + t.childState.name +
                          "' after " + str((time.time() - self.curState.startTimeSeconds) * 1000) + "ms")

                    self.curState = t.childState
                    self.curState.startTimeSeconds = time.time()

                    transitionMade = True

                    if t.onFire is not None:
                        t.onFire()

                    if self.curState.onEnter is not None:
                        self.curState.onEnter()

                    break # loop over transitions

            if not transitionMade:
                break

            loopCount += 1
            if loopCount > MAX_LOOP_COUNT:
                print("[FSMOption::runPolicy] Transition walk loop exceeded maximum number of iterations. Breaking from loop.")
                break

        return self.curState.options

