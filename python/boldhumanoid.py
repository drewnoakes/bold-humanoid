#!/usr/bin/env python3

import sys, getopt
import bold
import numpy as np

class PyConf(bold.ConfImpl):
    def getParamStr(self, path, defVal):
        return defVal
    def getParamInt(self, path, defVal):
        return defVal
    def getParamDbl(self, path, defVal):
        return defVal


def buildOptionTree():
    tree = bold.OptionTree()
    return tree

def thinkEndCallback():
    cameraState = bold.AgentState.getCameraFrameState()
    print(cameraState)
    print("Ball visible: ", cameraState.isBallVisible())
    ballObs = cameraState.getBallObservation()
    print(ballObs)
    goalObs = cameraState.getGoalObservations()
    print(goalObs)
    
def usage():
    print('''Options:
  -c <file>    select configuration file (or --conf)
  -t <num>     team number (or --team)
  -u <num>     uniform number (or --unum)
  -o           disable the option tree (or --no-tree)
  -g           disable auto get up from fallen (or --no-get-up)
  -j           allow control via joystick (or --joystick)
  -r           record one camera frame each second to PNG files (or --record)
  -h           show these options (or --help)''')

def main(argv):
    U2D_DEV_NAME = "/dev/ttyUSB0"
    MOTION_FILE_PATH = "./motion_4096.bin"
    CONF_FILE_PATH = "./germanopen.ini"
    TEAM_NUMBER = 24
    UNIFORM_NUMBER = -1
    USE_JOYSTICK = False
    AUTO_GET_UP = True
    USE_OPTION_TREE = False
    RECORD_FRAMES = False

    conf = PyConf()
    bold.Configurable.setConfImpl(conf)

    try:
        opts, args = getopt.getopt(argv,
                                   "c:t:u:ogjrh",
                                   ["conf=","team=","unum=","no-tree","no-get-up","joystick","record","help"])
    except getopt.GetoptError:
        print("ERROR: invalid options")
        usage()
        return

    for opt, arg in opts:
        if opt in ('-c', '--conf'):
            CONF_FILE_PATH = arg
        elif opt in ('-t', '--team'):
            TEAM_NUMBER = int(arg)
        elif opt in ('-u', '--unum'):
            UNIFORM_NUMBER = int(arg)
        elif opt in ('-o', '--no-tree'):
            USE_OPTION_TREE = False
        elif opt in ('-g', '--no-get-up'):
            AUTO_GET_UP = False
        elif opt in ('-j', '--joystick'):
            USE_JOYSTICK = True
        elif opt in ('-r', '--record'):
            RECORD_FRAMES = True
        elif opt in ('-h', '--help'):
            usage()
            return


    if UNIFORM_NUMBER < 0:
        print('ERROR: you must supply a uniform number')
        usage()
        return

    tree = buildOptionTree()

    a = bold.Agent(U2D_DEV_NAME,
                   CONF_FILE_PATH,
                   MOTION_FILE_PATH,
                   TEAM_NUMBER,
                   UNIFORM_NUMBER,
                   USE_JOYSTICK,
                   AUTO_GET_UP,
                   USE_OPTION_TREE,
                   RECORD_FRAMES)

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
        'shouldDrawObservedLines': False,
        'shouldDrawExpectedLines': False,
        'shouldDrawHorizon': True}

    visualCortex.set(**vcSettings)
                  
    #a.run()


if __name__ == "__main__":
    main(sys.argv[1:])

