#!/usr/bin/env python3

# Load basic modules
import sys, getopt
sys.path.append("swig")
import numpy as np
# Import Bold Hearts C++ library
import bold
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
    cameraState = bold.AgentState.getCameraFrameState()
    print("(Camera Frame) Ball visible: ", cameraState.isBallVisible())
    ballObs = cameraState.getBallObservation()
    print("(Camera Frame) Ball observations:")
    print(ballObs)

    agentFrameState = bold.AgentState.getAgentFrameState()
    print("(Agent Frame) Ball visible: ", agentFrameState.isBallVisible())
    ballObs = agentFrameState.getBallObservation()
    print("(Agent Frame) Ball observations:")
    print(ballObs)

    ambulatorState = bold.AgentState.getAmbulatorState()
    print(ambulatorState)
    print("AmbulatorState): current phase")
    print(ambulatorState.getCurrentPhase())

    print("=====")

    #goalObs = cameraState.getGoalObservations()
    #print(goalObs)


def usage():
    print('''Options:
  -c <file>    select configuration file (or --conf)
  -v           verbose logging (or --verbose)
  -h           show these options (or --help)
  --version    print git version details at time of build''')


def main(argv):
    configurationFile = "configuration-agent.json"
    bold.log.setMinLevelInfo()

    # Parse command arguments
    try:
        opts, args = getopt.getopt(argv,
                                   "c:vh",
                                   ["conf=","verbose","help","version"])
    except getopt.GetoptError:
        print("ERROR: invalid options")
        usage()
        return

    for opt, arg in opts:
        if opt in ('-c', '--conf'):
            configurationFile = arg
        elif opt in ('-v', '--verbose'):
            print("Setting log level to verbose")
            bold.log.setMinLevelVerbose()
        elif opt in ('-h', '--help'):
            usage()
            return
        elif opt in ('--version'):
            #TODO: implement
            print("Version reporting not yet implemented")
            return


    bold.Config.initialise("configuration-metadata.json", configurationFile)
    
    agent = getAgent()
    print(agent)

    bold.Config.initialisationCompleted();

    builder = PyOptionTreeBuilder()
    tree = builder.buildTree()
    agent.setOptionTree(tree);

    agent.onThinkEndConnect(thinkEndCallback);

    agent.run()

if __name__ == "__main__":
    main(sys.argv[1:])

