%{
#include <Option/MotionScriptOption/motionscriptoption.hh>
%}

namespace bold
{
  class MotionScriptOption : public bold::Option
  {
  public:
    MotionScriptOption(std::string const& id,
                       std::shared_ptr<bold::MotionScriptModule> motionScriptModule,
                       std::string const& fileName);

    virtual double hasTerminated();

    virtual std::vector<std::shared_ptr<Option> > runPolicy();
  };
}
