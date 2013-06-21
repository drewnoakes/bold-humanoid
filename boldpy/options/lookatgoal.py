import bold
from boldpy.agent import *
from numpy import *

class LookAtGoal(bold.Option):
    def __init__(self, id):
        super(LookAtGoal, self).__init__(id)

        self.gain = self.getParamDbl("gain", 0.85)
        self.maxOffset = self.getParamDbl("maxOffset", 20.0)

    def runPolicy(self):
        cameraState = bold.AgentState.getCameraFrameState()
        goalObs = cameraState.getGoalObservations()

        print(goalObs)

        if len(goalObs) < 2:
            print("[LookAtGoal::runPolicy] Couldn't see both goal posts!")
            return []

        middle = (goalObs[0] + goalObs[1]) / 2

        print("Middle: " + str(middle))

        # TODO: copied from LookAtBall; make into general function, or put getXenterPx on CameraModel
        # Or give HeadModule a method moveTrackPixel

        cm = getAgent().getCameraModel();
        w = cm.imageWidth()
        h = cm.imageHeight()

        centerPx = array([[w],
                          [h]]) / 2

        # Degrees per pixel
        happ = cm.rangeHorizontalDegs() / w
        vapp = cm.rangeVerticalDegs() / h

        offset = (middle - centerPx) * self.gain
        # multiplication is elementwise
        offset *= array([[happ],
                         [vapp]])

        # TODO: find or create saturate function
        offset = minimum(offset, array([[self.maxOffset],[self.maxOffset]]))
        offset = maximum(offset, array([[-self.maxOffset],[-self.maxOffset]]))

        hm = getAgent().getHeadModule()
        hm.moveTracking(offset[0,0], offset[1,0])

        return []
