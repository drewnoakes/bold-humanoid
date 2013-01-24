#include "Agent/agent.hh"

#define MOTION_FILE_PATH    "/darwin/Data/motion_4096.bin"
#define U2D_DEV_NAME0       "/dev/ttyUSB0"
#define U2D_DEV_NAME1       "/dev/ttyUSB1"

using namespace bold;

int main()
{
  Agent agent(U2D_DEV_NAME0, "config.ini", "/darwin/Data/motion_4096.bin");

  agent.run();
}
