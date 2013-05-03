#pragma once

#include "../option.hh"

namespace bold
{
  class Ambulator;

  class CircleBall : public Option
  {
  public:
    CircleBall(std::string const& id, std::shared_ptr<Ambulator> ambulator)
      : Option(id),
        d_ambulator(ambulator)
    {}

    OptionList runPolicy() override;

  private:
    std::shared_ptr<Ambulator> d_ambulator;
  };
}
