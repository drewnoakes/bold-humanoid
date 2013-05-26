#!/usr/bin/env python3

# Load basic modules
import sys, getopt
sys.path.append("swig")
import numpy as np

# Import C++ library
import bold

# Load configuraion module
from boldpy.conf import *

# Prepare configuration system
pc = PyConf(reportMissing = True)
bold.Configurable.setConfImpl(pc.__disown__())

# Load default paramters
import defparams as conf

# Load option tree building
from boldpy.optiontree import *

from boldpy.agent import *

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

conf.agent.add(agentParams)
conf.agent.testInt = 1
conf.agent.testDbl = 2.0
"""

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
            conf.agent.confFilePath = arg
        elif opt in ('-t', '--team'):
            conf.agent.teamNumber = int(arg)
        elif opt in ('-u', '--unum'):
            conf.agent.uniformNumber = int(arg)
        elif opt in ('-o', '--no-tree'):
            conf.agent.useOptionTree = False
        elif opt in ('-g', '--no-get-up'):
            conf.agent.autoGetUp = False
        elif opt in ('-j', '--joystick'):
            conf.agent.useJoystick = True
        elif opt in ('-r', '--record'):
            conf.agent.recordFrames = True
        elif opt == '--nogc':
            conf.agent.ignoreGameController = True
        elif opt in ('-h', '--help'):
            usage()
            return


    if conf.agent.uniformNumber < 0:
        print('ERROR: you must supply a uniform number')
        usage()
        return

    builder = PyOptionTreeBuilder()
    tree = builder.buildTree()
    agent.setOptionTree(tree)

    agent.onThinkEndConnect(thinkEndCallback);
    
    visualCortex = agent.getVisualCortex()
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
                  
    agent.run()
    
if __name__ == "__main__":
    main(sys.argv[1:])

