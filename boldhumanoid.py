#!/usr/bin/env python3

# Load basic modules
import sys, getopt
sys.path.append("swig")
import numpy as np
# Import C++ library
import bold
# Load configuraion module
import boldpy.conf as conf
# Load option tree module
from boldpy.options.optiontree import *
# Load agent module
from boldpy.agent import *
# Load interface to dynamically import modules
import importlib
# Catch signals
import signal
signal.signal(signal.SIGINT, signal.SIG_DFL)


def thinkEndCallback():
    print("===== HELLO =====")
    #cameraState = bold.AgentState.getCameraFrameState()
    #print(cameraState)
    #print("Ball visible: ", cameraState.isBallVisible())
    #ballObs = cameraState.getBallObservation()
    #print(ballObs)
    #goalObs = cameraState.getGoalObservations()
    #print(goalObs)


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
    global conf

    # Parse command arguments
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
            confFile = arg
            conf = importlib.import_module(confFile)
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
        elif opt in ('-h', '--help'):
            usage()
            return

    if conf.agent.uniformNumber < 0:
        print('ERROR: you must supply a uniform number')
        usage()
        return

    agent = getAgent()
    print(agent)

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

