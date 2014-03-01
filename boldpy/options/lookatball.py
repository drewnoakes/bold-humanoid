import bold
from boldpy.agent import *
from numpy import *

class LookAtBall(bold.Option):
    def __init__(self, id):
        super(LookAtBall, self).__init__(id)

        self.gain =      bold.Config.getDoubleSetting("options.look-at-ball.gain");
        self.maxOffset = bold.Config.getDoubleSetting("options.look-at-ball.max-offset-deg");

    def runPolicy(self):
        cameraState = bold.State.getCameraFrameState()
        ballObs = cameraState.getBallObservation()

        if ballObs is None:
            return []

        cm = getAgent().getCameraModel();
        w = cm.imageWidth()
        h = cm.imageHeight()

        centerPx = array([[w],
                          [h]]) / 2

        # Degrees per pixel
        happ = cm.rangeHorizontalDegs() / w
        vapp = cm.rangeVerticalDegs() / h

        offset = (ballObs - centerPx);
        # multiplication is elementwise
        offset *= array([[happ],
                         [vapp]])

        hm = getAgent().getHeadModule()
        hm.moveTracking(offset[0,0], offset[1,0])

        return []

