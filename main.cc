#include "Agent/agent.hh"
//#include "MotionScript/motionscript.hh"
#include "OptionTree/optiontree.hh"
#include "OptionTreeBuilder/AdHocOptionTreeBuilder/adhocoptiontreebuilder.hh"
// #include "RobotisMotionFile/robotismotionfile.hh"
//#include "ThreadId/threadid.hh"
#include "util/ccolor.hh"

#include "Config/config.hh"
#include <limits>

#include <signal.h>

using namespace bold;
using namespace std;

unique_ptr<Agent> agent;

void printUsage()
{
  cout << endl;
  cout << "Options:" << endl;
  cout << endl;
  cout << ccolor::fore::lightblue << "  -u <num> " << ccolor::fore::white << "uniform number (or --unum)" << endl;
  cout << ccolor::fore::lightblue << "  -t <num> " << ccolor::fore::white << "team number (or --team)" << endl;
  cout << ccolor::fore::lightblue << "  -h       " << ccolor::fore::white << "show these options (or --help)" << endl;
  cout << ccolor::reset;
}

void handleShutdownSignal(int sig)
{
  if (agent)
  {
    cout << "[boldhumanoid] Stopping Agent" << endl;
    agent->requestStop();
  }
}

// void convertMotionFile()
// {
//   vector<shared_ptr<MotionScript const>> motionScripts;
//
//   auto motionScriptFileName = "./motion_4096.bin";
//   cout << "[convertMotionFile] Processing Robotis-formatted motion file: " << motionScriptFileName << endl;
//   auto motionScriptFile = RobotisMotionFile(motionScriptFileName);
//   auto rootPageIndices = motionScriptFile.getSequenceRootPageIndices();
//   for (uchar rootPageIndex : rootPageIndices)
//   {
//     stringstream ss;
//     ss << "./motionscripts/motion_4096." << (int)rootPageIndex << ".json";
//     shared_ptr<MotionScript> motionScript = motionScriptFile.toMotionScript(rootPageIndex);
//     motionScripts.push_back(motionScript);
//     motionScript->writeJsonFile(ss.str());
//   }
//   motionScriptFile.toDotText(cout);
// }

int main(int argc, char **argv)
{
  cout << ccolor::bold << ccolor::fore::lightmagenta;
  cout << " _           _     _   _                     _       " << endl;
  cout << "| |         | |   | | | |                   | |      " << endl;
  cout << "| |__   ___ | | __| | | |__   ___  __ _ _ __| |_ ___ " << endl;
  cout << "| '_ \\ / _ \\| |/ _` | | '_ \\ / _ \\/ _` | '__| __/ __|" << endl;
  cout << "| |_) | (_) | | (_| | | | | |  __/ (_| | |  | |_\\__ \\" << endl;
  cout << "|_.__/ \\___/|_|\\__,_| |_| |_|\\___|\\__,_|_|   \\__|___/" << endl;
  cout << endl;
  cout << ccolor::reset;

  cout << "[boldhumanoid] Starting boldhumanoid" << endl;

//  convertMotionFile();

  // defaults
  unsigned teamNumber = 3; // team number in eindhoven, wc2013
  unsigned uniformNumber = 0;

  //
  // Process command line arguments
  //
  for (int i = 1; i < argc; ++i)
  {
    string arg(argv[i]);
    if (arg == "-h" || arg == "--help")
    {
      printUsage();
      return 0;
    }
    else if (arg == "-t" || arg == "--team")
    {
      teamNumber = atoi(argv[++i]);
    }
    else if (arg == "-u" || arg == "--unum")
    {
      uniformNumber = atoi(argv[++i]);
    }
    else
    {
      cout << ccolor::error << "UNKNOWN ARGUMENT: " << arg << ccolor::reset << endl;
      printUsage();
      return -1;
    }
  }

  if (uniformNumber == 0)
  {
    cout << ccolor::error << "YOU MUST SUPPLY A UNIFORM NUMBER!" << ccolor::reset << endl;
    printUsage();
    return -1;
  }

  Config::initialise("configuration-metadata.json", "configuration.json");

  cout << "[boldhumanoid] Creating Agent" << endl;
  cout << "[boldhumanoid] Team number " << teamNumber << ", uniform number " << uniformNumber << endl;

  agent.reset(new Agent());

  Config::initialisationCompleted();

  agent->setTeamNumber(teamNumber);
  agent->setUniformNumber(uniformNumber);

  AdHocOptionTreeBuilder optionTreeBuilder;
  auto optionTree = optionTreeBuilder.buildTree(teamNumber,
                                                uniformNumber,
                                                agent.get(),
                                                agent->getDataStreamer(),
                                                agent->getDebugger(),
                                                agent->getCameraModel(),
                                                agent->getAmbulator(),
                                                agent->getMotionScriptModule(),
                                                agent->getHeadModule(),
                                                agent->getWalkModule(),
                                                agent->getFallDetector());

  agent->setOptionTree(move(optionTree));

  signal(SIGTERM, &handleShutdownSignal);
  signal(SIGINT, &handleShutdownSignal);

  cout << "[boldhumanoid] Running Agent" << endl;
  agent->run();

  cout << "[boldhumanoid] Finished" << endl;

  return 0;
}
