import bold
import logging
import time
from boldpy.agent import *
from numpy import *
from .fsmoption import *
from .actionoption import *
from .stopwalking import *

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
        
        sw = StopWalking("stopwalking");
        tree.addOption(sw, True)

    def buildTree(self):
        tree = bold.OptionTree()

        self.createOptions(tree)

        # Prevent Python from deleting the tree (nicer to do when
        # actually passing it on to agent?)
        tree.thisown = 0
        return tree
