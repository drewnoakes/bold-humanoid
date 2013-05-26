%{
#include <Option/StopWalking/stopwalking.hh>
%}

namespace bold
{
  class StopWalking : public Option
  {
  public:
    StopWalking(std::string const& id, std::shared_ptr<Ambulator> ambulator);
  };
}
