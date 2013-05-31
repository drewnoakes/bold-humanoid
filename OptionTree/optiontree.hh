#pragma once

#include "../Option/option.hh"
#include <map>
#include <cassert>

namespace bold
{
  class OptionTree
  {
  public:
    void run();
    void addOption(std::shared_ptr<Option> option, bool top = false);
    std::shared_ptr<Option> getOption(std::string const& id) const;
    std::shared_ptr<Option> getTop() const;

  private:
    std::map<std::string, std::shared_ptr<Option> > d_options;
    std::shared_ptr<Option> d_top;
  };

  inline void OptionTree::addOption(std::shared_ptr<Option> option, bool top)
  {
    d_options[option->getID()] = option;

    if (top)
    {
      assert(!d_top && "top option already added");
      d_top = option;
    }
  }

  inline std::shared_ptr<Option> OptionTree::getOption(std::string const& id) const
  {
    auto option = d_options.find(id);
    if (option == d_options.end())
      return 0;
    return option->second;
  }

  inline std::shared_ptr<Option> OptionTree::getTop() const
  {
    return d_top;
  }
}
