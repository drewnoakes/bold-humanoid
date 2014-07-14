#include "lookaround.hh"

#include "../../Clock/clock.hh"
#include "../../MotionModule/HeadModule/headmodule.hh"
#include "../../State/state.hh"
#include "../../StateObject/CameraFrameState/cameraframestate.hh"

using namespace bold;
using namespace rapidjson;
using namespace std;

LookAround::LookAround(std::string const& id, std::shared_ptr<HeadModule> headModule, double sideAngle, std::function<double(uint)> speedCallback)
: Option(id, "LookAround"),
  d_speedCallback(speedCallback),
  d_headModule(headModule),
  d_sideAngle(sideAngle),
  d_topAngle(Config::getSetting<double>("options.look-around.top-angle")),
  d_bottomAngle(Config::getSetting<double>("options.look-around.bottom-angle")),
  d_durationHorizUpper(Config::getSetting<double>("options.look-around.horiz-duration-upper")),
  d_durationHorizLower(Config::getSetting<double>("options.look-around.horiz-duration-lower")),
  d_durationVert(Config::getSetting<double>("options.look-around.vert-duration")),
  d_speedStep(Config::getSetting<double>("options.look-around.speed-step")),
  d_isResetNeeded(true),
  d_lastTimeSeconds(0),
  d_speed(1.0),
  d_loopCount(0)
{}

vector<shared_ptr<Option>> LookAround::runPolicy(Writer<StringBuffer>& writer)
{
  // Make an oscillatory movement to search for the ball

  double t = Clock::getSeconds();

  double durationHorizUpper = d_durationHorizUpper->getValue();
  double durationHorizLower = d_durationHorizLower->getValue();
  double durationVert = d_durationVert->getValue();

  if (d_isResetNeeded)
  {
    // Start quarter-way through the first phase, so the head is slightly
    // to the left, and pans right through the top of the box.
    d_startTimeSeconds = t - (durationHorizUpper/4.0);
    d_isResetNeeded = false;
    writer.String("reset").Bool(true);
  }
  else if (d_speedCallback)
  {
    double speed = d_speed;
    double requestedSpeed = d_speedCallback(d_loopCount);
    // Smooth speed increase, with instant decrease
    speed += d_speedStep->getValue();
    if (requestedSpeed < speed)
      speed = requestedSpeed;
    speed = Math::clamp(speed, 0.0, 1.0);
    writer.String("speed").Double(speed);
    d_startTimeSeconds += (1 - speed) * (t - d_lastTimeSeconds);
    d_speed = speed;
  }

  writer.String("t").Double(t);

  d_lastTimeSeconds = t;

  double period = durationHorizUpper + durationHorizLower + (durationVert * 2);

  d_loopCount = (uint)((t - d_startTimeSeconds) / period);

  double phase = fmod(t - d_startTimeSeconds, period);

  double panDegs = 0;
  double tiltDegs = 0;

  writer.String("phase").Double(phase);

  ASSERT(phase >= 0);

  if (phase < durationHorizUpper)
  {
    // moving right-to-left across top
    tiltDegs = d_topAngle->getValue();
    panDegs = Math::lerp(phase/durationHorizUpper, -d_sideAngle, d_sideAngle);
    writer.String("stage").Int(1);
  }
  else
  {
    phase -= durationHorizUpper;

    if (phase < durationVert)
    {
      // moving top-to-bottom at left
      tiltDegs = Math::lerp(phase/durationVert, d_topAngle->getValue(), d_bottomAngle->getValue());
      panDegs = d_sideAngle;
      writer.String("stage").Int(2);
    }
    else
    {
      phase -= durationVert;

      if (phase < durationHorizLower)
      {
        // moving left-to-right across bottom
        tiltDegs = d_bottomAngle->getValue();
        panDegs = Math::lerp(phase/durationHorizLower, d_sideAngle, -d_sideAngle);
        writer.String("stage").Int(3);
      }
      else
      {
        phase -= durationHorizLower;

        if (phase <= durationVert)
        {
          // moving bottom-to-top at right
          tiltDegs = Math::lerp(phase/durationVert, d_bottomAngle->getValue(), d_topAngle->getValue());
          panDegs = -d_sideAngle;
          writer.String("stage").Int(4);
        }
        else
        {
          log::info("LookAround::runPolicy") << "Failed to find phase of motion";
          writer.String("stage").Int(-1);
        }
      }
    }
  }

  writer.String("pan").Double(panDegs);
  writer.String("tilt").Double(tiltDegs);

  // Move to the calculated position
  d_headModule->moveToDegs(panDegs, tiltDegs);

  return {};
}

void LookAround::reset()
{
  d_loopCount = 0;
  d_isResetNeeded = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

function<double(uint)> LookAround::speedIfBallVisible(double scaleWhenVisible, double scaleWhenNotVisible, double loopExp)
{
  return
    [scaleWhenVisible,scaleWhenNotVisible,loopExp]
    (uint loopCount)
    {
      return speedForLoop(loopCount, loopExp)
        * (State::get<CameraFrameState>()->isBallVisible() ? scaleWhenVisible : scaleWhenNotVisible);
    };
}

double LookAround::speedForLoop(uint loopCount, double loopExp)
{
  static auto defaultSpeedScalePerLoop = Config::getSetting<double>("options.look-around.default-speed-scale-per-loop");
  static auto minSpeedScalePerLoop = Config::getSetting<double>("options.look-around.min-speed-scale-per-loop");

  if (loopExp <= 0)
    loopExp = defaultSpeedScalePerLoop->getValue();

  return max(pow(loopExp, loopCount), minSpeedScalePerLoop->getValue());
}
