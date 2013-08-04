#include "motionscriptpage.hh"

#include "../JointId/jointid.hh"

using namespace bold;
using namespace std;

uchar MotionScriptPage::calculateChecksum() const
{
  uchar const* pt = (uchar const *)this;

  uchar checksum = 0x00;
  for (unsigned i = 0; i < sizeof(MotionScriptPage); i++)
  {
    checksum += *pt;
    pt++;
  }

  return checksum;
}

bool MotionScriptPage::isChecksumValid() const
{
  uchar calculated = calculateChecksum();

  // valid if the checksum calculation sums to 0xFF
  return calculated == 0xFF;
}

void MotionScriptPage::updateChecksum()
{
  // set to zero for the calculation
  checksum = 0x00;

  // calculate and update
  checksum = (uchar)(0xff - calculateChecksum());
}

void MotionScriptPage::reset()
{
  memset(this, 0, sizeof(MotionScriptPage));

  schedule = (uchar)MotionScriptPageSchedule::TIME_BASE; // default to time-base
  repeat = 1;
  speed = 32;
  accel = 32;

  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    slope[jointId] = 0x55;

  for (int i = 0; i < MAXNUM_STEPS; i++)
  {
    for (int j = 0; j < MAXNUM_POSITIONS; j++)
      steps[i].position[j] = INVALID_BIT_MASK;

    steps[i].pause = 0;
    steps[i].time = 0;
  }

  updateChecksum();
}
