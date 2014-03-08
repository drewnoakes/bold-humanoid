#pragma once

#include "../option.hh"
#include "../../CameraModel/cameramodel.hh"
#include "../../MotionModule/HeadModule/headmodule.hh"

namespace bold
{
  class LookAtGoal : public Option
  {
  public:
    LookAtGoal(std::string const& id, std::shared_ptr<CameraModel> cameraModel, std::shared_ptr<HeadModule> headModule)
    : Option(id, "LookAtGoal"),
      d_cameraModel(cameraModel),
      d_headModule(headModule)
    {}

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    virtual void reset() override { d_headModule->initTracking(); }

  private:
    std::shared_ptr<CameraModel> d_cameraModel;
    std::shared_ptr<HeadModule> d_headModule;
  };
}
