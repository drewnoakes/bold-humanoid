#pragma once

#include "../option.hh"

#include "../../Config/config.hh"

namespace bold
{
  class HeadModule;

  class LookAtFeet : public Option
  {
  public:
    LookAtFeet(std::string const& id, std::shared_ptr<HeadModule> headModule)
    : Option(id),
      d_headModule(headModule)
    {
      d_panDegs = Config::getSetting<double>("options.look-at-feet.head-pan-degs");
      d_tiltDegs = Config::getSetting<double>("options.look-at-feet.head-tilt-degs");
    }

    std::vector<std::shared_ptr<Option>> runPolicy() override;

  private:
    std::shared_ptr<HeadModule> d_headModule;
    Setting<double>* d_panDegs;
    Setting<double>* d_tiltDegs;
  };
}
