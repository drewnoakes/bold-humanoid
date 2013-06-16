import bold

class StopWalking(bold.Option):
    def __init__(self, id):
        bold.Option.__init__(self, id)
    
    def hasTerminated(self):
        return 1.0 if getAgent().getAmbulator().isRunning() else 0.0

    def runPolicy(self):
        amb = getAgent().getAmbulator()
        moveDir = array([[0],
                         [0]])
        amb.setMoveDir(moveDir)
        amd.setTurnAngle(0);
        return []

