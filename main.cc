#include "Agent/agent.hh"

#define MOTION_FILE_PATH    "/darwin/Data/motion_4096.bin"
#define U2D_DEV_NAME0       "/dev/ttyUSB0"
#define U2D_DEV_NAME1       "/dev/ttyUSB1"

using namespace bold;

int main(int argc, char **argv)
{
  bool showUI = false;
  bool useJoystick = false;

  //
  // Process command line arguments
  //
  std::vector<std::string> args(argv + 1, argv + argc);
  for (std::string arg : args) {
    if (arg == "-h" || arg == "--help") {
      std::cout << "Options:" << std::endl;
      std::cout << "\t-x\tshow graphical UI using X (or --gui)" << std::endl;
      std::cout << "\t-h\tshow these options (or --help)" << std::endl;
      return 0;
    } else if (arg == "-x" || arg == "--gui") {
      showUI = true;
    } else if (arg == "-j" || arg == "--joystick") {
      useJoystick = true;
    }
  }

  Agent agent(U2D_DEV_NAME0, "config.ini", "/darwin/Data/motion_4096.bin", showUI, useJoystick);

  return agent.run();
}
