import bold
from boldpy.agent import *
from numpy import *

class LookAtGoal(bold.Option):
    def __init__(self, id):
        super(LookAtGoal, self).__init__(id)

    def runPolicy(self):
        cameraState = bold.AgentState.getCameraFrameState()
        goalObs = cameraState.getGoalObservations()

        if len(goalObs) < 2:
            print("[LookAtGoal::runPolicy] Couldn't see both goal posts!")
            return []

        middle = (goalObs[0] + goalObs[1]) / 2

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

        offset = (middle - centerPx);
        # multiplication is elementwise
        offset *= array([[happ],
                         [vapp]])

        hm = getAgent().getHeadModule()
        hm.moveTracking(offset[0,0], offset[1,0])

        return []
