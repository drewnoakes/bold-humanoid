#pragma once

#include "../Option/option.hh"

namespace bold
{
  class SequenceOption : public Option
  {
  public:
    /// Make a shared_ptr of a SequenceOption. This offers a more convenient syntax than the alternative.
    static std::shared_ptr<SequenceOption> make(std::string const& id, std::initializer_list<std::shared_ptr<Option>> sequence)
    {
      return std::make_shared<SequenceOption>(id, sequence);
    };

    SequenceOption(std::string const& id, std::initializer_list<std::shared_ptr<Option>> sequence);

    void reset() override;

    double hasTerminated() override;

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

  private:
    std::vector<std::shared_ptr<Option>> d_sequence;
    int d_index;
  };
}
