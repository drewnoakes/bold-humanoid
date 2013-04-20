#ifndef BOLD_OPTIONTREESTATE_HH
#define BOLD_OPTIONTREESTATE_HH

#include "../stateobject.hh"
#include "../../Option/option.hh"

namespace bold
{
  class OptionTreeState : public StateObject
  {
  public:
    OptionTreeState(OptionList const& ranOptions)
    : d_ranOptions(ranOptions)
    {}

    OptionList getRanOptions() const { return d_ranOptions; };

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    OptionList d_ranOptions;
  };
}

#endif
