#pragma once

#include "../Option/option.hh"
#include "../Option/FSMOption/fsmoption.hh"
#include "../util/assert.hh"
#include "../util/log.hh"

#include <map>
#include <set>
#include <sstream>
#include <fstream>

namespace bold
{
  class OptionTree
  {
  public:
    void run();

    template<typename OptionType>
    std::shared_ptr<OptionType> addOption(std::shared_ptr<OptionType> option, bool isRoot = false)
    {
      d_options[option->getId()] = std::dynamic_pointer_cast<Option>(option);

      if (isRoot)
      {
        ASSERT(!d_root && "root option already added");
        d_root = std::dynamic_pointer_cast<Option>(option);
      }

      // Special handling for FSMOption
      auto fsm = std::dynamic_pointer_cast<FSMOption>(option);
      if (fsm)
      {
        // Validate the FSM
        if (!fsm->getStartState())
        {
          log::error("OptionTree::addOption") << "Attempt to add an FSMOption with ID '" << fsm->getId() << "' which has no start state";
          throw std::runtime_error("Attempt to add an FSMOption which has no start state");
        }

        // Write out its digraph to disk
        std::stringstream fileName;
        fileName << fsm->getId() << ".dot";

        std::ofstream winOut(fileName.str());
        winOut << fsm->toDot();
      }

      return option;
    }

    std::vector<std::shared_ptr<FSMOption>> getFSMs() const;

    unsigned optionCount() const { return d_options.size(); }

  private:
    std::map<std::string, std::shared_ptr<Option> > d_options;
    std::set<std::shared_ptr<Option>> d_optionsLastCycle;
    std::shared_ptr<Option> d_root;
  };
}
