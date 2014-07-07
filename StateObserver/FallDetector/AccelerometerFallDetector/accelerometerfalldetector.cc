#include "accelerometerfalldetector.hh"

#include "../../../CM730/cm730.hh"
#include "../../../Config/config.hh"
#include "../../../ThreadUtil/threadutil.hh"

using namespace bold;
using namespace std;

AccelerometerFallDetector::AccelerometerFallDetector(shared_ptr<Voice> voice)
: FallDetector(voice),
  TypedStateObserver<HardwareState>("Fall detector", ThreadId::MotionLoop),
  d_xAvg((ushort)Config::getStaticValue<int>("fall-detector.accelerometer.window-size")),
  d_yAvg((ushort)Config::getStaticValue<int>("fall-detector.accelerometer.window-size")),
  d_zAvg((ushort)Config::getStaticValue<int>("fall-detector.accelerometer.window-size"))
{}

void AccelerometerFallDetector::observeTyped(std::shared_ptr<HardwareState const> const& hardwareState, SequentialTimer& timer)
{
  // Track the smoothed acceleration along each axis to test for a consistent
  // indication that we have fallen.

  auto const& accRaw = hardwareState->getCM730State().accRaw;

  int xAvg = d_xAvg.next(accRaw.x()) - CM730::ACC_VALUE_MID;
  int yAvg = d_yAvg.next(accRaw.y()) - CM730::ACC_VALUE_MID;
  int zAvg = d_zAvg.next(accRaw.z()) - CM730::ACC_VALUE_MID;

  if (d_xAvg.isMature())
  {
    ASSERT(d_yAvg.isMature() && d_zAvg.isMature());

    if (abs(zAvg) > abs(xAvg) && abs(zAvg) > abs(yAvg))
    {
      // NOTE could actually be standing on our head, but this is quite unlikely :)
      setFallState(FallState::STANDUP);
    }
    else if (abs(xAvg) > abs(yAvg))
    {
      setFallState(xAvg < 0 ? FallState::RIGHT : FallState::LEFT);
    }
    else
    {
      setFallState(yAvg < 0 ? FallState::FORWARD : FallState::BACKWARD);
    }
  }
}

void AccelerometerFallDetector::logFallData(stringstream& msg) const
{
  msg << d_xAvg.getAverage() << ","
      << d_yAvg.getAverage() << ","
      << d_zAvg.getAverage();
}
