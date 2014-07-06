#pragma once

#include "../option.hh"
#include "../OdoWalkTo/odowalkto.hh"
#include "../../MotionModule/WalkModule/walkmodule.hh"

namespace bold
{
  class SquareDance : public Option
  {
  public:
    SquareDance(std::string const& id, std::shared_ptr<WalkModule> walkModule);

    OptionVector runPolicy() override;

  private:
    enum Stage
    {
      FORWARD,
      RIGHT,
      BACKWARD,
      LEFT,
      RESTART
    };
    Stage d_stage;
    std::shared_ptr<OdoWalkTo> d_odoWalkTo;
    
  };

}
