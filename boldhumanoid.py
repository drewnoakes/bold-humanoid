#!/usr/bin/env python3

import sys, getopt

sys.path.append("swig")
sys.path.append("build/swig")

import bold
from boldpy.conf import *
import numpy as np

# Prepare configuration system
pc = PyConf(reportMissing = True)
bold.Configurable.setConfImpl(pc.__disown__())

# Load default paramters
from defparams import *

print(agent.u2dDevName)

"""
agentParams = {
    "u2dDevName": "/dev/ttyUSB0",
    "motionFilePath": "./motion_4096.bin",
    "confFilePath": "./germanopen.ini",
    "teamNumber": 24,
    "uniformNumber": -1,
    "useJoystick": False,
    "autoGetUp": True,
    "useOptionTree": False,
    "recordFrames": False,
    "ignoreGameController": False
}

agent = Param(testStr="hello")
agent.add(agentParams)
agent.testInt = 1
agent.testDbl = 2.0
"""

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
	-o	     disable the option tree (or --no-tree)
        -g           disable auto get up from fallen (or --no-get-up)
	-j           allow control via joystick (or --joystick)
	-r           record one camera frame each second to PNG files (or --record)
	--nogc       do not listen to GameController
	-h           show these options (or --help)''')

agent.uniformNumber = -1

def main(argv):
    # Parse command arguments
    try:
        opts, args = getopt.getopt(argv,
                                   "c:t:u:ogjrh",
                                   ["conf=","team=","unum=","no-tree","no-get-up","joystick","record","nogc","help"])
    except getopt.GetoptError:
        print("ERROR: invalid options")
        usage()
        return

    for opt, arg in opts:
        if opt in ('-c', '--conf'):
            agent.confFilePath = arg
        elif opt in ('-t', '--team'):
            agent.teamNumber = int(arg)
        elif opt in ('-u', '--unum'):
            agent.uniformNumber = int(arg)
        elif opt in ('-o', '--no-tree'):
            agent.useOptionTree = False
        elif opt in ('-g', '--no-get-up'):
            agent.autoGetUp = False
        elif opt in ('-j', '--joystick'):
            agent.useJoystick = True
        elif opt in ('-r', '--record'):
            agent.recordFrames = True
        elif opt == '--nogc':
            agent.ignoreGameController = True
        elif opt in ('-h', '--help'):
            usage()
            return


    if agent.uniformNumber < 0:
        print('ERROR: you must supply a uniform number')
        usage()
        return

    tree = buildOptionTree()
    
    a = bold.Agent()

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
                  
    a.run()
    
if __name__ == "__main__":
    main(sys.argv[1:])

