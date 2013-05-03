#pragma once

#include "../option.hh"

namespace bold
{
  class Ambulator;

  class StopWalking : public Option
  {
  public:
    StopWalking(std::string const& id, std::shared_ptr<Ambulator> ambulator)
    : Option(id),
      d_ambulator(ambulator)
    {}

    double hasTerminated() override;

    OptionList runPolicy() override;

  private:
    std::shared_ptr<Ambulator> d_ambulator;
  };
}
