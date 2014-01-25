import bold
import logging
import time
from boldpy.agent import *
from numpy import *

from .fsmoption import *
from .motionscriptoption import *
from .stopwalking import *
from .lookatfeet import *
from .lookaround import *
from .lookatball import *
from .lookatgoal import *

class PyOptionTreeBuilder:
    def createMotionScriptOptions(self, tree, namesScripts):
        for ns in namesScripts:
            o = MotionScriptOption(ns[0], ns[1]).__disown__()
            tree.addOption(o)

    def createOptions(self, tree):
        # motionScriptOptions = [
        #     ("sitdownscript", "sit down"),
        #     ("standupscript", "stand up"),
        #     ("leftkickscript", "lk"),
        #     ("rightkickscript", "rk"),
        #     ("diveleftscript", "left_dive"),
        #     ("forwardgetupscript", "f up"),
        #     ("backwardgetupscript", "b up")]

        # self.createMotionScriptOptions(tree, motionScriptOptions)

        # self.sw = StopWalking("stopwalking").__disown__()
        # tree.addOption(self.sw)

        # self.laf = LookAtFeet("lookatfeet").__disown__()
        # tree.addOption(self.laf)

        self.lar = LookAround("lookaround").__disown__()
        tree.addOption(self.lar, True)

        # self.lab = LookAtBall("lookatball").__disown__()
        # tree.addOption(self.lab)

        # self.lag = LookAtGoal("lookatgoal").__disown__()
        # tree.addOption(self.lag)

        # self.createBootFSM(tree)
        # self.createWinFSM(tree)

    def createBootFSM(self, tree):
        bootFSM = FSMOption("bootfsm").__disown__()

        # STATES
        sitDownState = bootFSM.newState(options = [tree.getOption("sitdownscript")],
                                           startState = True,
                                           name = "sitdown")

        bootDoneState = bootFSM.newState(options = [],
                                         finalState = True,
                                         name = "bootdone")

        # TRANSITIONS
        sitDownState.\
            transitionTo(bootDoneState).\
            when(sitDownState.allOptionsTerminated)

        tree.addOption(bootFSM)

    def createPlayingFSM(self, tree):
        playingFSM = FSMOption("playing").__disown__()

        tree.addOption(playingFSM)

    def createWinFSM(self, tree):
        winFSM = FSMOption("winfsm").__disown__()

        # STATES
        bootingState = winFSM.newState(options = [tree.getOption("bootfsm")],
                                       startState = True,
                                       name = "bootfsm")

        pausingState = winFSM.newState(options = [tree.getOption("stopwalking")],
                                       name = "pausing")

        pausedState = winFSM.newState(options = [tree.getOption("sitdownscript")],
                                      name = "paused")

        unpausingState = winFSM.newState(options = [tree.getOption("standupscript")],
                                         name = "unpausing")

        readyState = winFSM.newState(options = [tree.getOption("stopwalking")],
                                     name = "ready")

        setState = winFSM.newState(options = [tree.getOption("stopwalking")],
                                   name = "set")


        beforeTheirKickoff = winFSM.newState(options = [tree.getOption("stopwalking")],
                                             name = "beforetheirkickoff")

        #playingState = winFsm.newState("playing", {playingFsm});

        penalizedState = winFSM.newState(options = [tree.getOption("stopwalking")],
                                         name = "penalized")

        forwardGetUpState = winFSM.newState(options= [tree.getOption("forwardgetupscript")],
                                                name ="forwardgetup")

        backwardGetUpState = winFSM.newState(options= [tree.getOption("backwardgetupscript")],
                                                name ="backwardgetup")

        dbg = getAgent().getDebugger()
        bootingState.onEnter = dbg.showPaused

        readyState.onEnter = dbg.showReady
        setState.onEnter = dbg.showSet
        #playingState.onEnter = dbg.showPlaying
        penalizedState.onEnter = dbg.showPenalized
        pausedState.onEnter = dbg.showPaused
        pausingState.onEnter = dbg.showPaused

        # TRANSITIONS

        def startButtonPressed():
            hw = bold.AgentState.getHardwareState()
            if hw is None:
                return False
            cm730 = hw.getCM730State()
            if cm730 is None:
                return False
            state = cm730.isStartButtonPressed
            print(state)
            if startButtonPressed.lastState is not state:
                startButtonPressed.lastState = state
                return state

            return False
        startButtonPressed.lastState = False

        bootingState.\
            transitionTo(pausedState).\
            when(bootingState.allOptionsTerminated)

        pausedState.\
            transitionTo(penalizedState).\
            when(startButtonPressed)

        tree.addOption(winFSM, True)

    def buildTree(self):
        tree = bold.OptionTree()

        self.createOptions(tree)

        # Prevent Python from deleting the tree (nicer to do when
        # actually passing it on to agent?)
        tree.thisown = 0
        return tree
