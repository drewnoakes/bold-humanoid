%{
#include <Debugger/debugger.hh>
%}

namespace bold
{
  class Debugger
  {
  public:
    void showReady();
    void showSet();
    void showPlaying();
    void showPenalized();
    void showPaused();
  };
}
