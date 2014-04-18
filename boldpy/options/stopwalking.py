import bold
from boldpy.agent import *
import numpy

class StopWalking(bold.Option):
    def __init__(self, id):
        bold.Option.__init__(self, id)

    def hasTerminated(self):
        return 1.0 if getAgent().getWalkModule().isRunning() else 0.0

    def runPolicy(self):
        walk = getAgent().getWalkModule()
        moveDir = numpy.array([[0.0],
                               [0.0]])
        walk.setMoveDir(moveDir)
        walk.setTurnAngle(0);
        return []
