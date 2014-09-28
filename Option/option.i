%{
#include <Option/option.hh>
%}

%feature("director") bold::Option;

namespace bold
{
  class Option
  {
  public:
    Option(std::string const& id);

    virtual ~Option();

    std::string getID() const;

    virtual double hasTerminated();

    virtual std::vector<std::shared_ptr<Option> > runPolicy();
  };
}
