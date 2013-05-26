import bold
from boldpy.agent import agent

class PyOptionTreeBuilder:
    def createActionOptions(self, tree, namesScripts):
        for ns in namesScripts:
            tree.addOption(bold.ActionOption(ns[0], ns[1], agent.getActionModule()))

    def createOptions(self, tree):
        actionOptions = [
            ("sitdownaction", "sit down"),
            ("standupaction", "stand up"),
            ("leftkickaction", "lk"),
            ("rightkickaction", "rk"),
            ("diveleftaction", "left_dive")]

        self.createActionOptions(tree, actionOptions)
        
        hm = agent.getHeadModule()
        wm = agent.getWalkModule()
        amb = agent.getAmbulator()

        tree.addOption(bold.StopWalking("stopwalking", amb))

    def buildTree(self):
        tree = bold.OptionTree()

        self.createOptions(tree)

        # Prefent Python from deleting the tree (nicer to do when
        # actually passing it on to agent?)
        tree.thisown = 0
        return tree
