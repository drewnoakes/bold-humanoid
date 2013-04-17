#ifndef BOLD_AGENTPOSITION_HH
#define BOLD_AGENTPOSITION_HH

namespace bold
{
  class AgentPosition
  {
  public:
    AgentPosition(double x, double y, double z, double theta)
    : d_x(x),
      d_y(y),
      d_z(z),
      d_theta(theta)
    {}

    double x() const { return d_x; }
    double y() const { return d_y; }
    double z() const { return d_z; }
    double theta() const { return d_theta; }

  private:
    double d_x;
    double d_y;
    double d_z;
    double d_theta;
  };
}

#endif