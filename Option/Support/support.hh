#pragma once

#include "../option.hh"
#include "../../Setting/setting.hh"
#include "../../MotionModule/WalkModule/walkmodule.hh"

namespace bold {
  
  class Support : public Option
  {
  public:
    Support(std::string const& id, std::shared_ptr<WalkModule> walkModule);

    OptionVector runPolicy() override;

  private:
    Setting<double>* d_yieldDistance; 

    std::shared_ptr<WalkModule> d_walkModule;
  };

  
}
