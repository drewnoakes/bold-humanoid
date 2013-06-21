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
            #("sitdownaction", "sit down"),
            ("standupaction", "stand up"),
            ("leftkickaction", "lk"),
            ("rightkickaction", "rk"),
            ("diveleftaction", "left_dive")]

        self.createActionOptions(tree, actionOptions)
        
        self.sit = ActionOption("sitdownaction", "sit down").__disown__()
        tree.addOption(self.sit)

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

    def createBootFSM(self, tree):
        bootFSM = FSMOption("bootfsm").__disown__()
        print(bootFSM)
        sitDownState = bootFSM.createAndAddState(options = [self.sit], startState = True)
        sitDownState.onEnter = lambda: print("Hello")

        print(sitDownState)
        lookAroundState = bootFSM.createAndAddState(options = [self.lar])
        print(lookAroundState)
        sitDownState.transitionTo(lookAroundState).when(sitDownState.allOptionsTerminated)
        print("a")
        tree.addOption(bootFSM, True)


    def buildTree(self):
        #print("buildTree: " + conf.confimpl.getParamStr("agent.u2dDevName", "not found"))
        tree = bold.OptionTree()

        self.createOptions(tree)

        # Prevent Python from deleting the tree (nicer to do when
        # actually passing it on to agent?)
        tree.thisown = 0
        return tree
