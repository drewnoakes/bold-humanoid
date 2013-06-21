import bold
from boldpy.agent import *
from numpy import *

class LookAtBall(bold.Option):
    def __init__(self, id):
        super(LookAtBall, self).__init__(id)

        self.gain =      self.getParamDbl("gain", 0.85)
        self.maxOffset = self.getParamDbl("maxOffset", 20.0)

    def runPolicy(self):
        cameraState = bold.AgentState.getCameraFrameState()
        ballObs = cameraState.getBallObservation()

        if ballObs is None:
            print("[LookAtBall::runPolicy] No ball seen!")
            return []

        cm = getAgent().getCameraModel();
        w = cm.imageWidth()
        h = cm.imageHeight()

        centerPx = array([[w],
                          [h]]) / 2

        # Degrees per pixel
        happ = cm.rangeHorizontalDegs() / w
        vapp = cm.rangeVerticalDegs() / h

        offset = (ballObs - centerPx) * self.gain
        # multiplication is elementwise
        offset *= array([[happ],
                         [vapp]])

        # TODO: find or create saturate function
        offset = minimum(offset, array([[self.maxOffset],[self.maxOffset]]))
        offset = maximum(offset, array([[-self.maxOffset],[-self.maxOffset]]))

        hm = getAgent().getHeadModule()
        hm.moveTracking(offset[0,0], offset[1,0])

        return []

