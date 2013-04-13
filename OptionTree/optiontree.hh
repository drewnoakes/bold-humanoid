#ifndef BOLD_OPTIONTREE_HH
#define BOLD_OPTIONTREE_HH

#include "../Option/option.hh"
#include <map>

namespace bold
{
  class OptionTree
  {
  public:
    void addOption(OptionPtr option, bool top = false);
    OptionPtr getOption(std::string const& id) const;
    OptionPtr getTop() const;

  private:
    std::map<std::string, OptionPtr> d_options;
    OptionPtr d_top;
  };


  inline void OptionTree::addOption(OptionPtr option, bool top)
  {
    d_options[option->getID()] = option;
    if (top)
      d_top = option;
  }

  inline OptionPtr OptionTree::getOption(std::string const& id) const
  { 
    auto option = d_options.find(id);
    if (option == d_options.end())
      return 0;
    return option->second;
  }

  inline OptionPtr OptionTree::getTop() const
  {
    return d_top;
  }
}

#endif
