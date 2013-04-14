#ifndef BOLD_APPROACHBALL_HH
#define BOLD_APPROACHBALL_HH

#include "../option.hh"

namespace bold
{
  class Ambulator;

  class ApproachBall : public Option
  {
  public:
    ApproachBall(std::string const& id, std::shared_ptr<Ambulator> ambulator)
      : Option(id),
        d_ambulator(ambulator)
    {}
        
    OptionList runPolicy() override;

  private:
    std::shared_ptr<Ambulator> d_ambulator;
  };
}

#endif
