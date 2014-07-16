#pragma once

namespace bold
{
  class Smoother
  {
  public:
    Smoother(double initialValue)
    : d_target(initialValue),
      d_current(initialValue),
      d_lastDelta(0.0)
    {}

    virtual ~Smoother() = default;

    virtual void step() = 0;

    void setTarget(double target)
    {
      d_target = target;
    }

    double getCurrent() const
    {
      return d_current;
    }

    double getTarget() const
    {
      return d_target;
    }

    double getNext()
    {
      auto prior = d_current;
      step();
      d_lastDelta = d_current - prior;
      return getCurrent();
    }

    double getLastDelta() const
    {
      return d_lastDelta;
    }

    void reset(double value = 0)
    {
      d_target = value;
      d_current = value;
      d_lastDelta = 0;
    }

  protected:
    double d_target;
    double d_current;
    double d_lastDelta;
  };
}
