#pragma once

#include "../option.hh"

namespace bold
{
  class CameraModel;
  class HeadModule;
  template<typename> class Setting;

  class LookAtBall : public Option
  {
  public:
    LookAtBall(std::string const& id, std::shared_ptr<CameraModel> cameraModel, std::shared_ptr<HeadModule> headModule);

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    virtual void reset() override;

  private:
    std::shared_ptr<CameraModel> d_cameraModel;
    std::shared_ptr<HeadModule> d_headModule;
    Setting<double>* d_gain;
    Setting<double>* d_minOffset;
    Setting<double>* d_maxOffset;
  };
}
