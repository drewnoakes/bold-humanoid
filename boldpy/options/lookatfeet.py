import bold
from boldpy.agent import *

class LookAtFeet(bold.Option):
    def __init__(self, id):
        super(LookAtFeet, self).__init__(id)

        self.panDegs = bold.Config.getDoubleSetting("options.look-at-feet.head-pan-degs")
        self.tiltDegs = bold.Config.getDoubleSetting("options.look-at-feet.head-tilt-degs")

    def runPolicy(self):
        hm = getAgent().getHeadModule()
        hm.moveToDegs(self.panDegs.getValue(), self.tiltDegs.getValue())

        return []

