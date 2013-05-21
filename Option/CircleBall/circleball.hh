#pragma once

#include "../option.hh"

namespace bold
{
  class Ambulator;
  class HeadModule;

  class CircleBall : public Option
  {
  public:
    CircleBall(std::string const& id, std::shared_ptr<Ambulator> ambulator, std::shared_ptr<HeadModule> headModule)
    : Option(id),
      d_ambulator(ambulator),
      d_headModule(headModule)
    {}

    OptionList runPolicy() override;

  private:
    std::shared_ptr<Ambulator> d_ambulator;
    std::shared_ptr<HeadModule> d_headModule;
  };
}
