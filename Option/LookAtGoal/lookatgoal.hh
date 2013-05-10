#pragma once

#include "../option.hh"
#include "../CameraModel/cameramodel.hh"

namespace bold
{
  class Head;

  class LookAtGoal : public Option
  {
  public:
    LookAtGoal(std::string const& id, std::shared_ptr<CameraModel> cameraModel, std::shared_ptr<Head> headModule)
    : Option(id),
      d_cameraModel(cameraModel),
      d_headModule(headModule)
    {}

    OptionList runPolicy() override;

  private:
    std::shared_ptr<CameraModel> d_cameraModel;
    std::shared_ptr<Head> d_headModule;
  };
}
