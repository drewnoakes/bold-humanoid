import bold
#from boldpy.agent import agent
import logging
import time

class ActionOption(bold.Option):

    """Option for playing actions in the motion file

    Runs the action with the name passed to the constructor, and
    reports terminated = 1.0 when the ActionModule is finished.

    """

    def __init__(self, id, actionName):
        """ActionOption constructor

        Extends the bold.Option constructor

        Args:
            id: A string giving the Option ID
            actionName: A string giving the name of a page in the action file
        """

        bold.Option.__init__(self, id)

        self.actionName = actionName
        self.started = False
        
    def hasTerminated(self):
        """ Return 1.0 if ActionModule is finshed, 0.0 otherwise. """

        if not self.started:
            return 0.0

        if (agent.getActionModule().isRunning()):
            return 0.0

        return 1.0;

    def runPolicy(self):
        """ Starts and runs action; returns empty list """

        if not self.started and not agent.getActionModule().isRunning:
            logging.info('Sarting action: ' + str(getID()))
            agent.getActionModule().start(self.actionName)
            self.started = true

        return [];


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
        startTime: The time, measured with time.time() in seconds,
          when this action became active.
    """

    def __init__(self, name, options = [], final = False):
        self.name = name
        self.options = options
        self.final = final
        self.transitions = []
        self.startTime = 0

    def secondsSinceStart(self):
        """Return the difference between now and start time. """
        return time.time() - self.startTime

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
            self.startState = startState

    def addTransition(self, transition):
        """Add a transition to this FSM
        
        Args:
          transition: FSMTransition object to add.
        """
        self.transitions.append(transition)


class PyOptionTreeBuilder:
    def createActionOptions(self, tree, namesScripts):
        for ns in namesScripts:
            o = ActionOption(ns[0], ns[1])
            o.thisown = 0
            tree.addOption(o)

    def createOptions(self, tree):
        actionOptions = [
            ("sitdownaction", "sit down"),
            ("standupaction", "stand up"),
            ("leftkickaction", "lk"),
            ("rightkickaction", "rk"),
            ("diveleftaction", "left_dive")]

        self.createActionOptions(tree, actionOptions)
        
    def buildTree(self):
        tree = bold.OptionTree()

        self.createOptions(tree)

        # Prevent Python from deleting the tree (nicer to do when
        # actually passing it on to agent?)
        tree.thisown = 0
        return tree
