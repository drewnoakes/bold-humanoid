import bold
from boldpy.agent import *
import time

class LookAround(bold.Option):
    def __init__(self, id):
        super(LookAround, self).__init__(id)

        self.lastTimeSeconds = 0
        self.startTimeSeconds = 0

        self.topAngle      = bold.Config.getDoubleSetting("options.look-around.top-angle");
        self.bottomAngle   = bold.Config.getDoubleSetting("options.look-around.bottom-angle");
        self.sideAngle     = 100 #bold.Config.getDoubleSetting("options.look-around.side-angle");
        self.durationHoriz = bold.Config.getDoubleSetting("options.look-around.horiz-duration");
        self.durationVert  = bold.Config.getDoubleSetting("options.look-around.vert-duration");

    def runPolicy(self):
        # Make an oscillatory movement to search for the ball
        t = time.time()

        if t - self.lastTimeSeconds > 1.0:
            # It's been long enough since we last ran that we consider this a re-start.
    
            # Start quarter-way through the first phase, so the head is slightly
            # to the left, and pans right through the top of the box.
            self.startTimeSeconds = t - self.durationHoriz.getValue() / 4.0

        self.lastTimeSeconds = t

        period = (self.durationHoriz.getValue() + self.durationVert.getValue()) * 2

        phase = (t - self.startTimeSeconds) % period
        hAngle = 0.0
        vAngle = 0.0

        def lerp(alpha, start, end):
            return (1.0 - alpha) * start + alpha * end

        if phase < self.durationHoriz.getValue():
            # Movinf right-to-left across top
            vAngle = self.topAngle.getValue()
            hAngle = lerp(phase / self.durationHoriz.getValue(), -self.sideAngle, self.sideAngle)
        else:
            phase -= self.durationHoriz.getValue()
            if phase < self.durationVert.getValue():
                # moving top-to-bottom at left
                vAngle = lerp(phase / self.durationVert.getValue(), self.topAngle.getValue(), self.bottomAngle.getValue())
                hAngle = self.sideAngle
            else:
                phase -= self.durationVert.getValue();
                if phase < self.durationHoriz.getValue():
                    # moving left-to-right across bottom
                    vAngle = self.bottomAngle.getValue()
                    hAngle = lerp(phase / self.durationHoriz.getValue(), self.sideAngle, -self.sideAngle)
                else:
                    phase -= self.durationHoriz.getValue()
                    if phase < self.durationVert.getValue():
                        # moving bottom-to-top at right
                        vAngle = lerp(phase / self.durationVert.getValue(), self.bottomAngle.getValue(), self.topAngle.getValue())
                        hAngle = -self.sideAngle
                    else:
                        print("[LookAround::runPolicy] Failed to find phase of motion")

        hm = getAgent().getHeadModule()
        hm.moveToDegs(hAngle, vAngle)
        
        return []

