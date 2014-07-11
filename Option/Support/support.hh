#pragma once

#include "../option.hh"
#include "../../Setting/setting.hh"

namespace bold {

  class WalkModule;
  class WalkTo;

  class Support : public Option
  {
  public:
    Support(std::string const& id, std::shared_ptr<WalkModule> walkModule);

    OptionVector runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

  private:
    Setting<double>* d_yieldDistance; 

    std::shared_ptr<WalkTo> d_walkTo;

  };

  
}
