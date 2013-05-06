#!/usr/bin/env python

import bold

def thinkEndCallback():
    cameraState = bold.AgentState.getCameraFrameState();
    print("Ball visible: ", cameraState.isBallVisible())

U2D_DEV_NAME = "/dev/ttyUSB0"
MOTION_FILE_PATH = "./motion_4096.bin"
CONF_FILE_PATH = "./config.ini"
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

a.run()
