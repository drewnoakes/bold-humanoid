%{
#include <MotionModule/motionmodule.hh>
#include <MotionModule/HeadModule/headmodule.hh>
#include <MotionModule/MotionScriptModule/motionscriptmodule.hh>
#include <MotionModule/WalkModule/walkmodule.hh>
#include <MotionScriptRunner/motionscriptrunner.hh>
%}

namespace bold
{
  class MotionModule
  {
  public:
    ~MotionModule();
  };

  class MotionScriptModule : public MotionModule
  {
  public:
    ~MotionScriptModule();

    bool start(std::shared_ptr<bold::MotionScriptRunner> runner);
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
