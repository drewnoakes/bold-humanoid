import bold
import logging
from boldpy.agent import *

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

        print("ActionOption.hasTerminated()")
        if not self.started:
            print("Not started yet; not terminated")
            return 0.0

        if (getAgent().getActionModule().isRunning()):
            print("Action module is running; not terminated")
            return 0.0

        print("Started and action module not running; terminated")
        return 1.0;

    def runPolicy(self):
        """ Starts and runs action; returns empty list """

        print("ActioNOption.runPolicy()")
        
        if not self.started and not getAgent().getActionModule().isRunning():
            print('Sarting action: ' + str(self.getID()))
            am = getAgent().getActionModule()
            res = am.start(self.actionName)
            print("Success" if res else "Fail")
            self.started = True
   
        return [];
