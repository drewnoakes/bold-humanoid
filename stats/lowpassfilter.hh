#pragma once

namespace bold
{
  class LowPassFilter
  {
  public:
    LowPassFilter() = default;
    void reset(double value) { d_value = value; }
    void setAlpha(double alpha) { d_alpha = alpha; }
    
    double next(double value) {
      d_value += d_alpha * (value - d_value);
      return d_value;
    }

    double getValue() const { return d_value; }
  private:
    double d_value;
    double d_alpha;
  };
}
