#pragma once

#include "../stateobject.hh"
#include <vector>
#include <memory>

namespace bold
{
  class Option;

  class OptionTreeState : public StateObject
  {
  public:
    // TODO is OptionList backed by a mutable object? if so, we may end up sending incorrect information to clients
    OptionTreeState(std::vector<std::shared_ptr<Option>> const& ranOptions)
    : d_ranOptions(ranOptions)
    {}

    std::vector<std::shared_ptr<Option>> getRanOptions() const { return d_ranOptions; };

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    std::vector<std::shared_ptr<Option>> d_ranOptions;
  };
}
