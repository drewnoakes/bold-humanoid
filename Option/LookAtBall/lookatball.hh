#pragma once

#include "../option.hh"

namespace bold
{
  class CameraModel;
  class HeadModule;

  class LookAtBall : public Option
  {
  public:
    LookAtBall(std::string const& id, std::shared_ptr<CameraModel> cameraModel, std::shared_ptr<HeadModule> headModule)
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
