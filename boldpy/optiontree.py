import bold
from boldpy.agent import agent
import logging

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
