%{
#include <Option/ActionOption/actionoption.hh>
%}

namespace bold
{
  class ActionOption : public Option
  {
  public:
    ActionOption(std::string const& id, std::string const& actionName);
  };
}
