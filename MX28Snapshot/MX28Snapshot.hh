#ifndef BOLD_MX28_SNAPSHOT_HH
#define BOLD_MX28_SNAPSHOT_HH

#include <CM730.h>

using namespace Robot;

class MX28Snapshot
{
private:
  unsigned short readTableWord(unsigned char* table, int addr);

public:
  MX28Snapshot() {}

  bool init(Robot::CM730 cm730, int mx82ID);
};

#endif