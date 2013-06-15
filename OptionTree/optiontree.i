%{
#include <OptionTree/optiontree.hh>
%}

%clearnodefaultctor;


namespace bold
{
  // Make Python disown Options automatically
  //%apply SWIGTYPE *DISOWN { Option* option_disowned };

  class OptionTree
  {
  public:
    void addOption(std::shared_ptr<Option> option, bool top = false);
    std::shared_ptr<Option> getOption(std::string const& id) const;
  };

}

%nodefaultctor;

