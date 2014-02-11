#include "jamdetector.hh"

#include "../../CM730Snapshot/cm730snapshot.hh"
#include "../../Config/config.hh"
#include "../../Voice/voice.hh"

using namespace bold;
using namespace std;

JamDetector::JamDetector(shared_ptr<Voice> voice)
: TypedStateObserver<BodyState>("Jam detector", ThreadId::MotionLoop),
  d_trackerByJointId(),
  d_voice(voice),
  d_enabled(Config::getSetting<bool>("hardware.jam-detector.enabled"))
{
  int lowThreshold  = Config::getStaticValue<int>("hardware.jam-detector.low-threshold");
  int highThreshold = Config::getStaticValue<int>("hardware.jam-detector.high-threshold");
  int windowSize    = Config::getStaticValue<int>("hardware.jam-detector.window-size");

  for (uchar jointId = (uchar)JointId::MIN; jointId <= (int)JointId::MAX; jointId++)
    d_trackerByJointId.emplace_back(lowThreshold, highThreshold, windowSize);
}

void JamDetector::observeTyped(std::shared_ptr<BodyState const> const& bodyState, SequentialTimer& timer)
{
  const auto& diffs = bodyState->getPositionValueDiffs();

  assert(diffs.size() == (int)JointId::MAX + 1);

  // TODO Don't just detect large error, but also the fact that the joint is hardly moving
  //      This is because during some motion script playback, there may be very large errors for a short while
  //      Alternative is to increase the window length

  for (uchar jointId = (uchar)JointId::MIN; jointId <= (int)JointId::MAX; jointId++)
  {
    int diff = diffs[jointId];
    int avg = d_trackerByJointId[jointId - 1].movingAverage.next(diff);

    if (!d_trackerByJointId[jointId - 1].movingAverage.isMature())
      continue;

    auto transition = d_trackerByJointId[jointId - 1].trigger.next(avg);

    if (transition == SchmittTriggerTransition::High)
    {
      log::warning("JamDetector::observeTyped") << "Prolonged deviation of joint " << (int)jointId << " (" << JointName::getName(jointId) << ") at avg level of " << avg << " over the last " << Config::getStaticValue<int>("hardware.jam-detector.window-size") << " cycles";
      d_voice->say("Help, I'm stuck!");

      // TODO actually trigger a defensive movement
    }
    else if (transition == SchmittTriggerTransition::Low)
    {
      log::info("JamDetector::observeTyped") << "Joint " << (int)jointId << " is now unstuck";
      d_voice->say("That's better");
    }
  }
}
