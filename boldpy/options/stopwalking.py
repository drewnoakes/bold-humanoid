import bold
from boldpy.agent import *
import numpy

class StopWalking(bold.Option):
    def __init__(self, id):
        bold.Option.__init__(self, id)
    
    def hasTerminated(self):
        print("StopWalking.hasTerminated()")
        return 1.0 if getAgent().getAmbulator().isRunning() else 0.0

    def runPolicy(self):
        print("StopWalking.runPolicy()")
        amb = getAgent().getAmbulator()
        moveDir = numpy.array([[0.0],
                               [0.0]])
        print(moveDir)
        print(amb.setMoveDir)
        amb.setMoveDir(moveDir)
        amb.setTurnAngle(0);
        return []
