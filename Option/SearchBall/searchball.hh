#pragma once

#include "../option.hh"

namespace bold
{
  class WalkModule;
  class HeadModule;

  class SearchBall : public Option
  {
  public:
    SearchBall(std::string const& id, std::shared_ptr<WalkModule> walkModule, std::shared_ptr<HeadModule> headModule)
    : Option(id, "SearchBall"),
      d_walkModule(walkModule),
      d_headModule(headModule),
      d_isLeftTurn(true),
      d_searchTop(true)
    {}

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    void setIsLeftTurn(bool leftTurn);

    virtual void reset() override;

  private:
    std::shared_ptr<WalkModule> d_walkModule;
    std::shared_ptr<HeadModule> d_headModule;
    bool d_isLeftTurn;
    bool d_searchTop;
  };
}
