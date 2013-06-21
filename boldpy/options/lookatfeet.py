import bold
from boldpy.agent import *

class LookAtFeet(bold.Option):
    def __init__(self, id):
        super(LookAtFeet, self).__init__(id)

        self.feetX = self.getParamDbl("feetX", 0.0)
        self.feetY = self.getParamDbl("feetY", -67.5)

    def runPolicy(self):
        hm = getAgent().getHeadModule()
        hm.moveToDegs(self.feetX, self.feetY)

        return []

