import bold
from boldpy.agent import agent
import logging
import time

class ActionOption(bold.Option):
    def __init__(self, id, actionName):
        bold.Option.__init__(self, id)

        self.actionName = actionName
        self.started = False

        
    def hasTerminated(self):
        if not self.started:
            return 0.0

        if (agent.getActionModule().isRunning()):
            return 0.0

        return 1.0;


    def runPolicy(self):
        if not self.started and not agent.getActionModule().isRunning:
            logging.info('Sarting action: ' + str(getID()))
            agent.getActionModule().start(self.actionName)
            self.started = true

        return [];


class FSMTransition:
    def __init__(self, name, parentState = None):
        self.name = name
        
        self.condition = None
        self.onFire = None
        self.parentState = parentState
        self.childState = None
        

class FSMState:
    def __init__(self, name, options = [], final = False):
        self.name = name
        self.options = options
        self.final = final
        self.transitions = []
        self.startTime = 0

    def secondsSinceStart(self):
        return time.time() - self.startTime

    def allOptionsTerminated(self):
        return all(o.hasTerminated() for o in self.options)

    def newTransition(self, name = ""):
        t = FSMTransition(name, self)
        self.transitions.append(t)
        return t

        
class FSMOption(bold.Option):
    def __init__(self, id):
        bold.Option.__init__(self, id)
        self.states = []
        self.transitions = []

        self.startState = None
        self.curState = None

    def isAvailable(self):
        return True

    def hasTerminated(self):
        if not self.curState is None and self.curState.final:
            return 1.0
        else:
            return 0.0

    def addState(self, state, startState = False):
        self.states.append(state)
        if startState:
            self.startState = startState

    def addTransition(self, transition):
        self.transitions.append(transition)


class PyOptionTreeBuilder:
    def createActionOptions(self, tree, namesScripts):
        for ns in namesScripts:
            tree.addOption(ActionOption(ns[0], ns[1]))

    def createOptions(self, tree):
        actionOptions = [
            ("sitdownaction", "sit down"),
            ("standupaction", "stand up"),
            ("leftkickaction", "lk"),
            ("rightkickaction", "rk"),
            ("diveleftaction", "left_dive")]

        self.createActionOptions(tree, actionOptions)
        
        #hm = agent.getHeadModule()
        #wm = agent.getWalkModule()
        #amb = agent.getAmbulator()

        #tree.addOption(bold.StopWalking("stopwalking", amb))

    def buildTree(self):
        tree = bold.OptionTree()

        self.createOptions(tree)

        # Prefent Python from deleting the tree (nicer to do when
        # actually passing it on to agent?)
        tree.thisown = 0
        return tree
