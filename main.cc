#include "Agent/agent.hh"
#include "OptionTree/optiontree.hh"

#include <signal.h>

#define MOTION_FILE_PATH    "./motion_4096.bin"
#define U2D_DEV_NAME0       "/dev/ttyUSB0"
#define U2D_DEV_NAME1       "/dev/ttyUSB1"

using namespace bold;
using namespace std;

Agent* agent;

void handleShutdownSignal(int sig)
{
  if (agent)
    agent->stop();
}

int main(int argc, char **argv)
{
  cout << "[boldhumanoid] Starting boldhumanoid" << endl;

  // defaults
  bool useJoystick = false;
  bool autoGetUpFromFallen = true;
  bool recordFrames = false;
  bool useOptionTree = true;
  unsigned uniformNumber = 0;

  string confFile("config.ini");

  //
  // Process command line arguments
  //
  for (int i = 1; i < argc; ++i)
  {
    string arg(argv[i]);
    if (arg == "-h" || arg == "--help")
    {
      cout << "Options:" << endl;
      cout << "\t-c <file>\tselect configuration file (or --conf)" << endl;
      cout << "\t-n <num>\tplayer number (or --number)" << endl;
      cout << "\t-t\tdisable the option tree (or --no-tree)" << endl;
      cout << "\t-g\tdisable auto get up from fallen (or --no-get-up)" << endl;
      cout << "\t-j\tallow control via joystick (or --joystick)" << endl;
      cout << "\t-r\trecord one camera frame each second to PNG files (or --record)" << endl;
      cout << "\t-h\tshow these options (or --help)" << endl;
      return 0;
    }
    else if (arg == "-c" || arg == "--conf")
    {
      confFile = argv[++i];
    }
    else if (arg == "-n" || arg == "--number")
    {
      uniformNumber = atoi(argv[++i]);
    }
    else if (arg == "-j" || arg == "--joystick")
    {
      useJoystick = true;
    }
    else if (arg == "-g" || arg == "--no-get-up")
    {
      autoGetUpFromFallen = false;
    }
    else if (arg == "-t" || arg == "--no-tree")
    {
      useOptionTree = false;
    }
    else if (arg == "-r" || arg == "--record")
    {
      recordFrames = true;
    }
  }

  minIni ini(confFile);

  agent = new Agent(
    U2D_DEV_NAME0,
    ini,
    MOTION_FILE_PATH,
    9, // Team number. TODO: set sensible
    uniformNumber,
    useJoystick,
    autoGetUpFromFallen,
    useOptionTree,
    recordFrames);

  signal(SIGTERM, &handleShutdownSignal);
  signal(SIGINT, &handleShutdownSignal);

  agent->run();

  cout << "[boldhumanoid] Finished" << endl;

  return 0;
}
