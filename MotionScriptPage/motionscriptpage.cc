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
  d_checksum = 0x00;

  // calculate and update
  d_checksum = (uchar)(0xff - calculateChecksum());
}

void MotionScriptPage::reset()
{
  memset(this, 0, sizeof(MotionScriptPage));

  d_schedule = (uchar)MotionScriptPageSchedule::TIME_BASE; // default to time-base
  d_repeatCount = 1;
  d_speed = DEFAULT_SPEED;
  d_accelerationTime = DEFAULT_ACCELERATION;

  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    d_slopes[jointId] = DEFAULT_SLOPE;

  for (int i = 0; i < MAXNUM_STEPS; i++)
  {
    for (int j = 0; j < MAXNUM_POSITIONS; j++)
      d_steps[i].position[j] = INVALID_BIT_MASK;

    d_steps[i].pause = 0;
    d_steps[i].time = 0;
  }

  updateChecksum();
}
