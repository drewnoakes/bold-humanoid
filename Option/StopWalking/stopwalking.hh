#pragma once

#include "../option.hh"

namespace bold
{
  class Ambulator;

  class StopWalking : public Option
  {
  public:
    StopWalking(std::string const& id, std::shared_ptr<Ambulator> ambulator)
    : Option(id, "StopWalking"),
      d_ambulator(ambulator)
    {}

    double hasTerminated() override;

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

  private:
    std::shared_ptr<Ambulator> d_ambulator;
  };
}
