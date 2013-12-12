#include "ambulator.hh"

#include "../MotionModule/WalkModule/walkmodule.hh"
#include "../Config/config.hh"

using namespace bold;
using namespace std;

Ambulator::Ambulator(shared_ptr<WalkModule> walkModule)
  : d_walkModule(walkModule),
    d_xAmp(0, Config::getValue<double>("ambulator.x-amp-delta")),
    d_yAmp(0, Config::getValue<double>("ambulator.y-amp-delta")),
    d_turnAmp(0, Config::getValue<double>("ambulator.turn-delta")),
    d_maxHipPitchAtSpeed(Config::getSetting<double>("ambulator.max-hip-pitch-at-speed")),
    d_minHipPitch(Config::getSetting<double>("ambulator.min-hip-pitch")),
    d_maxHipPitch(Config::getSetting<double>("ambulator.max-hip-pitch")),
    d_turnAngleSet(false),
    d_moveDirSet(false)
{
  auto xAmpSetting = Config::getSetting<double>("ambulator.x-amp-delta");
  auto yAmpSetting = Config::getSetting<double>("ambulator.y-amp-delta");
  auto turnSetting = Config::getSetting<double>("ambulator.turn-delta");

  xAmpSetting->changed.connect([this](double value) { d_yAmp.setDelta(value); });
  yAmpSetting->changed.connect([this](double value) { d_yAmp.setDelta(value); });
  turnSetting->changed.connect([this](double value) { d_turnAmp.setDelta(value); });
}
