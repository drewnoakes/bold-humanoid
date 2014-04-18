#pragma once

#include "../option.hh"

namespace bold
{
  class WalkModule;

  class StopWalking : public Option
  {
  public:
    StopWalking(std::string const& id, std::shared_ptr<WalkModule> walkModule)
    : Option(id, "StopWalking"),
      d_walkModule(walkModule)
    {}

    double hasTerminated() override;

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

  private:
    std::shared_ptr<WalkModule> d_walkModule;
  };
}
