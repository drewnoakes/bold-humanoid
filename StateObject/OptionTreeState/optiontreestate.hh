#pragma once

#include "../stateobject.hh"
#include "../../Option/option.hh"

namespace bold
{
  class OptionTreeState : public StateObject
  {
  public:
    // TODO is OptionList backed by a mutable object? if so, we may end up sending incorrect information to clients
    OptionTreeState(OptionList const& ranOptions)
    : d_ranOptions(ranOptions)
    {}

    OptionList getRanOptions() const { return d_ranOptions; };

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    OptionList d_ranOptions;
  };
}
