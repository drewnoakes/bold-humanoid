#ifndef BOLD_LOOKATBALL_HH
#define BOLD_LOOKATBALL_HH

#include "../option.hh"
#include "../CameraModel/cameramodel.hh"

namespace bold
{
  class LookAtBall : public Option
  {
  public:
    LookAtBall(std::string const& id, std::shared_ptr<CameraModel> cameraModel)
    : Option(id),
      d_cameraModel(cameraModel)
    {}

    OptionPtr runPolicy();

  private:
    std::shared_ptr<CameraModel> d_cameraModel;
  };
}

#endif
