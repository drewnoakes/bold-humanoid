%{
#include <Option/ActionOption/actionoption.hh>
%}

namespace bold
{
  class ActionPage;

  class ActionOption : public Option
  {
  public:
    ActionOption(std::string const& id, std::string const& actionName, std::shared_ptr<ActionModule> actionModule);

    ActionOption(std::string const& id, ActionPage actionPage, std::shared_ptr<ActionModule> actionModule);
  };
}
