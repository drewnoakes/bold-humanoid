#pragma once

#include "../Option/option.hh"

#include <map>
#include <set>

namespace bold
{
  class FSMOption;

  class OptionTree
  {
  public:
    OptionTree(std::shared_ptr<Option> root);

    void run();

    void registerFsm(std::shared_ptr<FSMOption> fsm);

    std::vector<std::shared_ptr<FSMOption>> getFSMs() const;

  private:
    std::map<std::string, std::shared_ptr<FSMOption>> d_fsmOptions;
    std::set<std::shared_ptr<Option>> d_optionsLastCycle;
    const std::shared_ptr<Option> d_root;
  };
}
