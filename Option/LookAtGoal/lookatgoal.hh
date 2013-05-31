#pragma once

#include "../option.hh"
#include "../CameraModel/cameramodel.hh"

namespace bold
{
  class HeadModule;

  class LookAtGoal : public Option
  {
  public:
    LookAtGoal(std::string const& id, std::shared_ptr<CameraModel> cameraModel, std::shared_ptr<HeadModule> headModule)
    : Option(id),
      d_cameraModel(cameraModel),
      d_headModule(headModule)
    {}

    std::vector<std::shared_ptr<Option>> runPolicy() override;

  private:
    std::shared_ptr<CameraModel> d_cameraModel;
    std::shared_ptr<HeadModule> d_headModule;
  };
}
