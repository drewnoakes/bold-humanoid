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

    unsigned optionCount() const { return d_options.size(); }

  private:
    std::map<std::string, std::shared_ptr<Option> > d_options;
    std::shared_ptr<Option> d_top;
  };

  inline std::shared_ptr<Option> OptionTree::getTop() const
  {
    return d_top;
  }
}
