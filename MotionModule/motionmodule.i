%{
#include <MotionModule/motionmodule.hh>
#include <MotionModule/ActionModule/actionmodule.hh>
#include <MotionModule/HeadModule/headmodule.hh>
#include <MotionModule/WalkModule/walkmodule.hh>
%}

namespace bold
{
  class MotionModule
  {
  public:
    ~MotionModule();
  };

  class ActionModule : public MotionModule
  {
  public:
    ~ActionModule();

    bool start(std::string namePage);
    bool isRunning();
  };

  class HeadModule : public MotionModule
  {
  public:
    ~HeadModule();

    void moveToHome();
    void moveToDegs(double panDegs, double tiltDegs);
    void moveByDeltaDegs(double panDegsDelta, double tiltDegsDelta);
    void initTracking();
    void moveTracking(double panError, double tiltError);
  };

  class WalkModule : public MotionModule
  {
  public:
    ~WalkModule();
  };
}
