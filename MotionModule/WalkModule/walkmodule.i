%{
#include <Ambulator/ambulator.hh>
%}

namespace bold
{
  class Ambulator
  {
  public:
    void stop();
    bool isRunning() const;
    void setMoveDir(Eigen::Vector2d const& moveDir);
    void setTurnAngle(double turnSpeed);
  };
}
