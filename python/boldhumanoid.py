#!/usr/bin/env python

import bold
import numpy as np

def thinkEndCallback():
    cameraState = bold.AgentState.getCameraFrameState()
    print(cameraState)
    print("Ball visible: ", cameraState.isBallVisible())
    ballObs = cameraState.getBallObservation()
    print(ballObs)
    goalObs = cameraState.getGoalObservations()
    print(goalObs)
    
U2D_DEV_NAME = "/dev/ttyUSB0"
MOTION_FILE_PATH = "./motion_4096.bin"
CONF_FILE_PATH = "./germanopen.ini"
TEAM_NUMBER = 24
UNIFORM_NUMBER = 2
USE_JOYSTICK = False
AUTO_GET_UP = True
USE_OPTION_TREE = False
RECORD_FRAMES = False
IGNORE_GAME_CONTROLLER = True

a = bold.Agent(U2D_DEV_NAME, 
               CONF_FILE_PATH,
               MOTION_FILE_PATH,
               TEAM_NUMBER,
               UNIFORM_NUMBER,
               USE_JOYSTICK,
               AUTO_GET_UP,
               USE_OPTION_TREE,
               RECORD_FRAMES,
               IGNORE_GAME_CONTROLLER)

a.onThinkEndConnect(thinkEndCallback);

visualCortex = a.getVisualCortex()
visualCortex.setShouldIgnoreAboveHorizon(False)

vcSettings = {
    'shouldDetectLines': False,
    'shouldIgnoreAboveHorizon': False,
    'minBallArea': 8,
    'streamFramePeriod': 5,
    'shouldDrawBlobs': True,
    'shouldDrawLineDots': True,
    'shouldDrawExpectedLines': False,
    'shouldDrawHorizon': True}

visualCortex.set(**vcSettings)
                  

a.run()
