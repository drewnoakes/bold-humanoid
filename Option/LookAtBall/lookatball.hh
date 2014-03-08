#pragma once

#include "../option.hh"
#include "../../Config/config.hh"
#include "../../MotionModule/HeadModule/headmodule.hh"

namespace bold
{
  class CameraModel;
  template<typename> class Setting;

  class LookAtBall : public Option
  {
  public:
    LookAtBall(std::string const& id, std::shared_ptr<CameraModel> cameraModel, std::shared_ptr<HeadModule> headModule)
    : Option(id, "LookAtBall"),
      d_cameraModel(cameraModel),
      d_headModule(headModule),
      d_gain(Config::getSetting<double>("options.look-at-ball.gain")),
      d_minOffset(Config::getSetting<double>("options.look-at-ball.offset-min")),
      d_maxOffset(Config::getSetting<double>("options.look-at-ball.offset-max"))
    {}

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    virtual void reset() override { d_headModule->initTracking(); }

  private:
    std::shared_ptr<CameraModel> d_cameraModel;
    std::shared_ptr<HeadModule> d_headModule;
    Setting<double>* d_gain;
    Setting<double>* d_minOffset;
    Setting<double>* d_maxOffset;
  };
}
