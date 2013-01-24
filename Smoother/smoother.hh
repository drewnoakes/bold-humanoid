#ifndef BOLD_SMOOTHER_HH
#define BOLD_SMOOTHER_HH

namespace bold
{
  class Smoother
  {
  protected:
    double d_target;
    double d_current;

  public:
    Smoother(double initialValue)
    : d_current(initialValue),
      d_target(initialValue)
    {}

    virtual void step() = 0;

    void setTarget(double target)
    {
      d_target = target;
    }

    double getCurrent()
    {
      return d_current;
    }

    double getTarget()
    {
      return d_target;
    }

    double getNext()
    {
      step();
      return getCurrent();
    }
  };
}

#endif