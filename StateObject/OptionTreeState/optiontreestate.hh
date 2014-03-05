#pragma once

#include "../stateobject.hh"

#include <rapidjson/document.h>
#include <vector>
#include <memory>

namespace bold
{
  class Option;

  class OptionTreeState : public StateObject
  {
  public:
    // TODO is OptionList backed by a mutable object? if so, we may end up sending incorrect information to clients
    OptionTreeState(std::vector<std::shared_ptr<Option>> const& ranOptions, std::unique_ptr<rapidjson::Document> optionJson)
    : d_ranOptions(ranOptions),
      d_optionJson(std::move(optionJson))
    {}

    std::vector<std::shared_ptr<Option>> getRanOptions() const { return d_ranOptions; };

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    std::vector<std::shared_ptr<Option>> d_ranOptions;
    std::unique_ptr<rapidjson::Document> d_optionJson;
  };
}
