#pragma once

#include "../option.hh"

namespace bold
{
  class Ambulator;
  class HeadModule;

  class SearchBall : public Option
  {
  public:
    SearchBall(std::string const& id, std::shared_ptr<Ambulator> ambulator, std::shared_ptr<HeadModule> headModule)
    : Option(id, "SearchBall"),
      d_ambulator(ambulator),
      d_headModule(headModule),
      d_isLeftTurn(true)
    {}

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    void setIsLeftTurn(bool leftTurn);

    virtual void reset() override;

  private:
    std::shared_ptr<Ambulator> d_ambulator;
    std::shared_ptr<HeadModule> d_headModule;
    bool d_isLeftTurn;
  };
}
