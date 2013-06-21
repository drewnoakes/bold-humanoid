import bold
from boldpy.agent import *
import time

class LookAround(bold.Option):
    def __init__(self, id):
        super(LookAround, self).__init__(id)

        self.lastTimeSeconds = 0
        self.startTimeSeconds = 0

        self.topAngle      = self.getParamDbl("topAngle",     30.0)
        self.bottomAngle   = self.getParamDbl("bottomAngle", -25.0)
        self.sideAngle     = self.getParamDbl("sideAngle",   100.0)
        self.durationHoriz = self.getParamDbl("durationHoriz", 2.3)
        self.durationVert  = self.getParamDbl("durationVert",  0.2)

    def runPolicy(self):
        # Make an oscillatory movement to search for the ball
        t = time.time()

        if t - self.lastTimeSeconds > 1.0:
            # It's been long enough since we last ran that we consider this a re-start.
    
            # Start quarter-way through the first phase, so the head is slightly
            # to the left, and pans right through the top of the box.
            self.startTimeSeconds = t - self.durationHoriz / 4.0

        self.lastTimeSeconds = t

        period = (self.durationHoriz + self.durationVert) * 2

        phase = (t - self.startTimeSeconds) % period
        hAngle = 0.0
        vAngle = 0.0

        def lerp(alpha, start, end):
            return (1.0 - alpha) * start + alpha * end

        if phase < self.durationHoriz:
            # Movinf right-to-left across top
            vAngle = self.topAngle
            hAngle = lerp(phase / self.durationHoriz, -self.sideAngle, self.sideAngle)
        else:
            phase -= self.durationHoriz
            if phase < self.durationVert:
                # moving top-to-bottom at left
                vAngle = lerp(phase / self.durationVert, self.topAngle, self.bottomAngle)
                hAngle = self.sideAngle
            else:
                phase -= self.durationVert
                if phase < self.durationHoriz:
                    # moving left-to-right across bottom
                    vAngle = self.bottomAngle
                    hAngle = lerp(phase / self.durationHoriz, self.sideAngle, -self.sideAngle)
                else:
                    phase -= self.durationHoriz
                    if phase < self.durationVert:
                        # moving bottom-to-top at right
                        vAngle = lerp(phase / self.durationVert, self.bottomAngle, self.topAngle)
                        hAngle = -self.sideAngle
                    else:
                        print("[LookAround::runPolicy] Failed to find phase of motion")

        hm = getAgent().getHeadModule()
        hm.moveToDegs(hAngle, vAngle)
        
        return []

