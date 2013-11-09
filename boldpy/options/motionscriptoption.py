import bold
import logging
from boldpy.agent import *

class MotionScriptOption(bold.Option):

    """Option for playing motion scripts

    Runs the motion script with the name passed to the constructor, and
    reports terminated = 1.0 when the MotionScriptModule is finished.

    """

    def __init__(self, id, motionScriptName):
        """MotionScriptOption constructor

        Extends the bold.Option constructor

        Args:
            id: A string giving the Option ID
            motionScriptName: A string giving the name of the motion script to run
        """

        bold.Option.__init__(self, id)

        self.motionScriptName = motionScriptName
        self.started = False

    def hasTerminated(self):
        """ Return 1.0 if MotionScriptModule is finshed, 0.0 otherwise. """

        print("MotionScriptOption.hasTerminated()")
        if not self.started:
            print("Not started yet; not terminated")
            return 0.0

        if (getAgent().getMotionScriptModule().isRunning()):
            print("MotionScriptModule is running; not terminated")
            return 0.0

        print("Started and MotionScriptModule not running; terminated")
        return 1.0;

    def runPolicy(self):
        """ Starts and runs motion script; returns empty list """

        print("MotionScriptOption.runPolicy()")

        if not self.started and not getAgent().getMotionScriptModule().isRunning():
            print('Sarting motion script: ' + str(self.getID()))
            am = getAgent().getMotionScriptModule()
            res = am.start(self.motionScriptName)
            print("Success" if res else "Fail")
            self.started = True

        return [];
