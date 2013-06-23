import bold
import logging
import time
from boldpy.agent import *
from numpy import *

from .fsmoption import *
from .actionoption import *
from .stopwalking import *
from .lookatfeet import *
from .lookaround import *
from .lookatball import *
from .lookatgoal import *

#import boldpy.conf as conf

class PyOptionTreeBuilder:
    def createActionOptions(self, tree, namesScripts):
        for ns in namesScripts:
            o = ActionOption(ns[0], ns[1]).__disown__()
            tree.addOption(o)

    def createOptions(self, tree):
        actionOptions = [
            ("sitdownaction", "sit down"),
            ("standupaction", "stand up"),
            ("leftkickaction", "lk"),
            ("rightkickaction", "rk"),
            ("diveleftaction", "left_dive"),
            ("forwardgetupaction", "f up"),
            ("backwardgetupaction", "b up")]

        self.createActionOptions(tree, actionOptions)
        
        self.sw = StopWalking("stopwalking").__disown__()
        tree.addOption(self.sw)

        self.laf = LookAtFeet("lookatfeet").__disown__()
        tree.addOption(self.laf)

        self.lar = LookAround("lookaround").__disown__()
        tree.addOption(self.lar)

        self.lab = LookAtBall("lookatball").__disown__()
        tree.addOption(self.lab)

        self.lag = LookAtGoal("lookatgoal").__disown__()
        tree.addOption(self.lag)

        self.createBootFSM(tree)
        self.createWinFSM(tree)

    def createBootFSM(self, tree):
        bootFSM = FSMOption("bootfsm").__disown__()

        # STATES
        sitDownState = bootFSM.newState(options = [tree.getOption("sitdownaction")],
                                           startState = True,
                                           name = "sitdown")
        standUpState = bootFSM.newState(options = [tree.getOption("standupaction")],
                                           name = "standup")

        lookAroundState = bootFSM.newState(options = [self.lar],
                                              name = "lookaround")

        # TRANSITIONS
        sitDownState.\
            transitionTo(standUpState).\
            when(sitDownState.allOptionsTerminated)

        standUpState.\
            transitionTo(lookAroundState).\
            when(standUpState.allOptionsTerminated)

        tree.addOption(bootFSM)

    def createWinFSM(self, tree):
        winFSM = FSMOption("winfsm").__disown__()

        # STATES
        bootingState = winFSM.newState(options = [tree.getOption("bootfsm")],
                                       startState = True,
                                       name = "winfsm")
        
        pausingState = winFSM.newState(options = [tree.getOption("stopwalking")],
                                       name = "pausing")
        
        pausedState = winFSM.newState(options = [tree.getOption("sitdownaction")],
                                      name = "paused")

        unpausingState = winFSM.newState(options = [tree.getOption("standupaction")],
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

        forwardGetUpState = winFSM.newState(options= [tree.getOption("forwardgetupaction")],
                                                name ="forwardgetup")
  
        backwardGetUpState = winFSM.newState(options= [tree.getOption("backwardgetupaction")],
                                                name ="backwardgetup")

        dbg = getAgent().getDebugger()

        tree.addOption(winFSM, True)
  
    def buildTree(self):
        #print("buildTree: " + conf.confimpl.getParamStr("agent.u2dDevName", "not found"))
        tree = bold.OptionTree()

        self.createOptions(tree)

        # Prevent Python from deleting the tree (nicer to do when
        # actually passing it on to agent?)
        tree.thisown = 0
        return tree
