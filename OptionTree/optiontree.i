%{
#include <OptionTree/optiontree.hh>
%}

%clearnodefaultctor;

namespace bold
{
  class OptionTree
  {
    std::shared_ptr<Option> getOption(std::string const& id) const;
  };

  %extend OptionTree
  {
    void addOption(Option* option, bool top = false)
    {
      $self->addOption(std::shared_ptr<bold::Option>(option), top);
    }
  }
}

%nodefaultctor;

