#pragma once

#include <memory>
#include <string>

#include "../MotionModule/motionmodule.hh"
#include "../SequentialTimer/sequentialtimer.hh"

namespace bold
{
  class CM730;

  class CM730CommsModule
  {
  public:
    CM730CommsModule(std::string const& name)
    : d_name(name)
    {}

    virtual ~CM730CommsModule() {}

    std::string getName() const { return d_name; }

    virtual void step(std::unique_ptr<CM730>& cm730, SequentialTimer& t, ulong motionCycleNumber) = 0;

  private:
    std::string d_name;
  };
}
