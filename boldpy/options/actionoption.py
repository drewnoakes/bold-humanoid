import bold

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

        if not self.started:
            return 0.0

        if (getAgent().getActionModule().isRunning()):
            return 0.0

        return 1.0;

    def runPolicy(self):
        """ Starts and runs action; returns empty list """

        if not self.started and not getAgent().getActionModule().isRunning:
            logging.info('Sarting action: ' + str(getID()))
            getAgent().getActionModule().start(self.actionName)
            self.started = true

        return [];
