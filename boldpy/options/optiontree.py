import bold
from boldpy import *
from boldpy.agent import *
from numpy import *

from .fsmoption import *
from .stopwalking import *
from .lookatfeet import *
from .lookaround import *
from .lookatball import *
from .lookatgoal import *

class PyOptionTreeBuilder:
    def createMotionScriptOptions(self, tree, namesScripts):
        for ns in namesScripts:
            o = bold.MotionScriptOption(ns[0], getAgent().getMotionScriptModule(), "./motionscripts/" + ns[1] + ".json")
            tree.addOption(o)

    def createOptions(self, tree):
        motionScriptOptions = [
            ("leftKickScript", "kick-left"),
            ("rightKickScript", "kick-right"),
            ("diveLeftScript", "dive-left"),
            ("diveRightScript", "dive-right"),
            ("forwardGetUpScript", "get-up-from-front"),
            ("backwardGetUpScript", "get-up-from-back"),
            ("bigStepLeftScript", "step-left-big"),
            ("bigStepRightScript", "step-right-big"),
            ("standUpScript", "stand-ready"),
            ("sitDownScript", "sit-down")]
        self.createMotionScriptOptions(tree, motionScriptOptions)

        self.lar = LookAround("lookAround").__disown__()
        tree.addOption(self.lar)

        self.lab = LookAtBall("lookAtBall").__disown__()
        tree.addOption(self.lab, True)

        self.laf = LookAtFeet("lookAtFeet").__disown__()
        tree.addOption(self.laf)

        self.sw = StopWalking("stopWalking").__disown__()
        tree.addOption(self.sw)

        self.lag = LookAtGoal("lookAtGoal").__disown__()
        tree.addOption(self.lag)

        self.createWinFSM(tree)
        # self.createWinFSM(tree)

    def createWinFSM(self, tree):
        winFSM = FSMOption("win").__disown__()

        # GENERAL FUNCTIONS

        @static_var("lastState", False)
        def startButtonPressed():
            hw = bold.State.getHardwareState()
            if not hw:
                return False
            cm730 = hw.getCM730State()
            if startButtonPressed.lastState != cm730.isStartButtonPressed:
                startButtonPressed.lastState = cm730.isStartButtonPressed
                return startButtonPressed.lastState;
            return False

        @static_var("lastState", False)
        def modeButtonPressed():
            hw = bold.State.getHardwareState()
            if not hw:
                return False
            cm730 = hw.getCM730State()
            if modeButtonPressed.lastState != cm730.isModeButtonPressed:
                modeButtonPressed.lastState = cm730.isModeButtonPressed
                return modeButtonPressed.lastState;
            return False

        def negate(func):
            return lambda: not func()

        # STATES
        startUpState = winFSM.newState(options = [tree.getOption("sitDownScript")],
                                       startState = True,
                                       name = "startUp")
        readyState = winFSM.newState(options = [tree.getOption("stopWalking")],
                                     name = "ready")
        pausing1State = winFSM.newState(options = [tree.getOption("stopWalking")],
                                        name = "pausing1")
        pausing2State = winFSM.newState(options = [tree.getOption("sitDownScript")],
                                        name = "pausing2")
        pausedState = winFSM.newState(options = [],
                                      name = "paused")
        unpausingState = winFSM.newState(options = [tree.getOption("standUpScript")],
                                         name = "unpausing")
        setState = winFSM.newState(options = [tree.getOption("stopWalking")],
                                   name = "set")
        beforeTheirKickoffState = winFSM.newState(options = [tree.getOption("stopWalking")],
                                                  name = "beforeTheirKickoff")
        playingState = winFSM.newState(options = [],
                                       name = "playing")
        penalizedState = winFSM.newState(options = [tree.getOption("stopWalking")],
                                         name = "penalized")
        forwardGetUpState = winFSM.newState(options = [tree.getOption("forwardGetUpScript")],
                                            name = "forwardGetUp")
        backwardGetUpState = winFSM.newState(options = [tree.getOption("backwardGetUpScript")],
                                             name = "backwardGetUp")
        stopWalkingForShutdownState = winFSM.newState(options = [tree.getOption("stopWalking")],
                                                      name = "stopWalkingForShutdown")
        sitForShutdownState = winFSM.newState(options = [tree.getOption("sitDownScript")],
                                              name = "sitForShutdown")
        stopAgentAndExitState = winFSM.newState(options = [],
                                                name = "stopAgentAndExit")


        dbg = getAgent().getDebugger()
        hm = getAgent().getHeadModule()

        readyState.onEnter = lambda: [dbg.showReady(), hm.moveToHome()]
        setState.onEnter = lambda: [dbg.showSet(), hm.moveToHome()]
        playingState.onEnter = dbg.showPlaying
        penalizedState.onEnter = lambda: [dbg.showPenalized(), hm.moveToHome()]
        pausedState.onEnter = dbg.showPaused
        pausing1State.onEnter = lambda: [dbg.showPaused(), hm.moveToHome()]
        stopAgentAndExitState.onEnter = getAgent().stop

        # TRANSITIONS
        startUpState.\
            transitionTo(readyState).\
            whenTerminated()

        pausedState.\
            transitionTo(unpausingState).\
            when(startButtonPressed)

        unpausingState.\
            transitionTo(setState).\
            whenTerminated()

        playingState.\
            transitionTo(pausing1State).\
            when(startButtonPressed)

        pausing1State.\
            transitionTo(pausing2State).\
            when(negate(getAgent().getWalkModule().isRunning))

        pausing2State.\
            transitionTo(pausedState).\
            whenTerminated()

        readyState.\
            transitionTo(setState).\
            when(modeButtonPressed)

        setState.\
            transitionTo(penalizedState).\
            when(modeButtonPressed)

        penalizedState.\
            transitionTo(playingState).\
            when(modeButtonPressed)

        tree.addOption(winFSM, True)

    def buildTree(self):
        tree = bold.OptionTree()

        self.createOptions(tree)

        # Prevent Python from deleting the tree (nicer to do when
        # actually passing it on to agent?)
        tree.thisown = 0
        return tree
