#include "lookaround.hh"

#include "../../Config/config.hh"
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
  d_speedStep(Config::getSetting<double>("options.look-around.speed-step")),
  d_lastTime(0),
  d_lastSpeed(1.0),
  d_loopCount(0)
{
  double topAngle           = Config::getStaticValue<double>("options.look-around.top-angle");
  double bottomAngle        = Config::getStaticValue<double>("options.look-around.bottom-angle");
  double durationHorizUpper = Config::getStaticValue<double>("options.look-around.horiz-duration-upper");
  double durationHorizLower = Config::getStaticValue<double>("options.look-around.horiz-duration-lower");
  double durationVert       = Config::getStaticValue<double>("options.look-around.vert-duration");

  d_stages.emplace_back(durationHorizUpper, topAngle,    -sideAngle);
  d_stages.emplace_back(durationVert,       topAngle,     sideAngle);
  d_stages.emplace_back(durationHorizLower, bottomAngle,  sideAngle);
  d_stages.emplace_back(durationVert,       bottomAngle, -sideAngle);

  reset();
}

vector<shared_ptr<Option>> LookAround::runPolicy(Writer<StringBuffer>& writer)
{
  // Make an oscillatory movement to search for the ball

  auto t = Clock::getTimestamp();

  if (d_lastTime != 0)
  {
    double speed = 1.0;

    // Modify speed according to the callback, if present
    if (d_speedCallback)
    {
      double requestedSpeed = d_speedCallback(d_loopCount);

      // Smooth speed increase
      speed = d_lastSpeed + d_speedStep->getValue();

      // If the requested speed is lower than the current speed, drop to it immediately
      if (requestedSpeed < speed)
        speed = requestedSpeed;

      // Clamp speed within range [0,1]
      speed = Math::clamp(speed, 0.0, 1.0);

      writer.String("speed");
      writer.Double(speed, "%.3f");
    }

    // Progress the phase according to the elapsed time
    double deltaSeconds = Clock::timestampToSeconds(t - d_lastTime);
    double deltaPhase = deltaSeconds / d_stages[(int)d_phase].durationSeconds;
    // NOTE this may spill some of the time from one stage into another, but in reality that's fine
    d_phase += deltaPhase * speed;

    if (d_phase > d_stages.size())
    {
      // We completed a cycle
      d_loopCount++;
      d_phase -= d_stages.size();
    }

    d_lastSpeed = speed;
  }

  d_lastTime = t;

  int fromStageIndex = (int)d_phase;
  int toStageIndex = (int)((fromStageIndex + 1) % d_stages.size());
  double stageRatio = fmod(d_phase, 1.0);

  ASSERT(d_phase >= 0);
  ASSERT(d_phase < d_stages.size());

  writer.String("phase");
  writer.Double(d_phase, "%.3f");
  writer.String("fromStageIndex");
  writer.Int(fromStageIndex);
  writer.String("toStageIndex");
  writer.Int(toStageIndex);
  writer.String("stageRatio");
  writer.Double(stageRatio, "%.3f");

  auto const& fromStage = d_stages[fromStageIndex];
  auto const& toStage = d_stages[toStageIndex];

  double panDegs  = Math::lerp(stageRatio, fromStage.panAngle,  toStage.panAngle);
  double tiltDegs = Math::lerp(stageRatio, fromStage.tiltAngle, toStage.tiltAngle);

  writer.String("pan");
  writer.Double(panDegs, "%.3f");
  writer.String("tilt");
  writer.Double(tiltDegs, "%.3f");

  // Move to the calculated position
  d_headModule->moveToDegs(panDegs, tiltDegs);

  return {};
}

void LookAround::reset()
{
  d_loopCount = 0;
  d_lastTime = 0;
  d_lastSpeed = 1.0;

  // Start quarter-way through the first stage, so the head is slightly
  // to the left, and pans right through the top of the box.
  d_phase = 0.25;
}

void LookAround::setPhase(double phase)
{
  ASSERT(phase >= 0);
  ASSERT(phase < d_stages.size());
  d_phase = phase;
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
