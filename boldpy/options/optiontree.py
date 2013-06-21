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
        
        sit = ActionOption("sitdownaction", "sit down").__disown__()
        tree.addOption(sit)

        sw = StopWalking("stopwalking").__disown__()
        tree.addOption(sw)

        laf = LookAtFeet("lookatfeet").__disown__()
        tree.addOption(laf)

        lar = LookAround("lookaround").__disown__()
        tree.addOption(lar, True)

    def buildTree(self):
        #print("buildTree: " + conf.confimpl.getParamStr("agent.u2dDevName", "not found"))
        tree = bold.OptionTree()

        self.createOptions(tree)

        # Prevent Python from deleting the tree (nicer to do when
        # actually passing it on to agent?)
        tree.thisown = 0
        return tree
