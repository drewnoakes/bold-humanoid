#include "Agent/agent.hh"
#include "MotionScript/motionscript.hh"
#include "OptionTree/optiontree.hh"
#include "OptionTreeBuilder/AdHocOptionTreeBuilder/adhocoptiontreebuilder.hh"
#include "RobotisMotionFile/robotismotionfile.hh"
#include "ThreadId/threadid.hh"
#include "util/ccolor.hh"

#include <signal.h>

#define MOTION_FILE_PATH    "./motion_4096.bin"
#define U2D_DEV_NAME0       "/dev/ttyUSB0"
#define U2D_DEV_NAME1       "/dev/ttyUSB1"

using namespace bold;
using namespace std;

unique_ptr<Agent> agent;

void printUsage()
{
  cout << "Options:" << endl;
  cout << "\t-t <num>\tteam number (or --team)" << endl;
  cout << "\t-u <num>\tuniform number (or --unum)" << endl;
  cout << "\t-t\tdisable the option tree (or --no-tree)" << endl;
  cout << "\t-g\tdisable auto get up from fallen (or --no-get-up)" << endl;
  cout << "\t-j\tallow control via joystick (or --joystick)" << endl;
  cout << "\t-r\trecord one camera frame each second to PNG files (or --record)" << endl;
  cout << "\t-h\tshow these options (or --help)" << endl;
}

void handleShutdownSignal(int sig)
{
  if (agent)
  {
    cout << "[boldhumanoid] Stopping Agent" << endl;
    agent->requestStop();
  }
}

void convertMotionFile()
{
  vector<shared_ptr<MotionScript const>> motionScripts;

  auto motionScriptFileName = "./motion_4096.bin";
  cout << "[convertMotionFile] Processing Robotis-formatted motion file: " << motionScriptFileName << endl;
  auto motionScriptFile = RobotisMotionFile(motionScriptFileName);
  auto rootPageIndices = motionScriptFile.getSequenceRootPageIndices();
  for (uchar rootPageIndex : rootPageIndices)
  {
    stringstream ss;
    ss << "./motionscripts/motion_4096." << (int)rootPageIndex << ".json";
    shared_ptr<MotionScript> motionScript = motionScriptFile.toMotionScript(rootPageIndex);
    motionScripts.push_back(motionScript);
    motionScript->writeJsonFile(ss.str());
  }
  motionScriptFile.toDotText(cout);
}

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

  Configurable::setConfImpl(new ConfImpl());

  // defaults
//   bool useJoystick = false;
//   bool autoGetUpFromFallen = true;
//   bool recordFrames = false;
//   bool useOptionTree = true;
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
//     else if (arg == "-j" || arg == "--joystick")
//     {
//       useJoystick = true;
//     }
//     else if (arg == "-g" || arg == "--no-get-up")
//     {
//       autoGetUpFromFallen = false;
//     }
//     else if (arg == "-t" || arg == "--no-tree")
//     {
//       useOptionTree = false;
//     }
//     else if (arg == "-r" || arg == "--record")
//     {
//       recordFrames = true;
//     }
    else
    {
      cout << "UNKNOWN ARGUMENT: " << arg << endl;
      printUsage();
      return -1;
    }
  }

  if (uniformNumber == 0)
  {
    cout << "YOU MUST SUPPLY A UNIFORM NUMBER!" << endl;
    printUsage();
    return -1;
  }

  cout << "[boldhumanoid] Creating Agent" << endl;
  cout << "[boldhumanoid] Team number " << teamNumber << ", uniform number " << uniformNumber << endl;

  agent.reset(new Agent());

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
