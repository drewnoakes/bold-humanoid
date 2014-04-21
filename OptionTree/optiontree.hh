#pragma once

#include "../Option/option.hh"
#include "../util/assert.hh"

#include <map>
#include <set>

namespace bold
{
  // TODO rename 'top' as 'root'?

  class FSMOption;

  class OptionTree
  {
  public:
    void run();

    template<typename OptionType>
    std::shared_ptr<OptionType> addOption(std::shared_ptr<OptionType> option, bool isTop = false)
    {
      d_options[option->getId()] = std::dynamic_pointer_cast<Option>(option);

      if (isTop)
      {
        ASSERT(!d_top && "top option already added");
        d_top = std::dynamic_pointer_cast<Option>(option);
      }

      return option;
    }

    std::shared_ptr<Option> getOption(std::string const& id) const;
    std::shared_ptr<Option> getTop() const;
    std::vector<std::shared_ptr<FSMOption>> getFSMs() const;

    unsigned optionCount() const { return d_options.size(); }

  private:
    std::map<std::string, std::shared_ptr<Option> > d_options;
    std::set<std::shared_ptr<Option>> d_optionsLastCycle;
    std::shared_ptr<Option> d_top;
  };

  inline std::shared_ptr<Option> OptionTree::getTop() const
  {
    return d_top;
  }
}
