#ifndef BOLD_STAND_HH
#define BOLD_STAND_HH

#include "../option.hh"

namespace bold
{
  class Ambulator;

  class Stand : public Option
  {
  public:
    Stand(std::string const& id, std::shared_ptr<Ambulator> ambulator)
      : Option(id),
	d_ambulator(ambulator)
    {}

    OptionList runPolicy() override;

  private:
    std::shared_ptr<Ambulator> d_ambulator;
  };
}

#endif
