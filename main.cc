#include "Agent/agent.hh"
#include "WorldModel/worldmodel.hh"

#define MOTION_FILE_PATH    "./motion_4096.bin"
#define U2D_DEV_NAME0       "/dev/ttyUSB0"
#define U2D_DEV_NAME1       "/dev/ttyUSB1"

using namespace bold;
using namespace std;

int main(int argc, char **argv)
{
  bool showUI = false;
  bool useJoystick = false;
  bool autoGetUpFromFallen = true;

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
      cout << "\t-c <conffile>\tselect configuration file (or --conf)" << endl;
      cout << "\t-x\tshow graphical UI using X (or --gui)" << endl;
      cout << "\t-j\tallow control via joystick (or --joystick)" << endl;
      cout << "\t-g\tdisable auto get up from fallen (or --no-get-up)" << endl;
      cout << "\t-h\tshow these options (or --help)" << endl;
      return 0;
    }
    else if (arg == "-c" || arg == "--conf")
    {
      confFile = argv[++i];
    }
    else if (arg == "-x" || arg == "--gui")
    {
      showUI = true;
    }
    else if (arg == "-j" || arg == "--joystick")
    {
      useJoystick = true;
    }
    else if (arg == "-g" || arg == "--no-get-up")
    {
      autoGetUpFromFallen = false;
    }
  }

  minIni ini(confFile);

  VisualCortex::getInstance().initialise(ini);
  WorldModel::getInstance().initialise(ini);

  Agent agent(
    U2D_DEV_NAME0,
    ini,
    MOTION_FILE_PATH,
    showUI,
    useJoystick,
    autoGetUpFromFallen);

  return agent.run();
}
