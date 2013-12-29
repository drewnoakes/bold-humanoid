#pragma once

#include <stdexcept>
#include "log.hh"

namespace bold
{
  enum class SchmittTriggerTransition
  {
    None,
    High,
    Low
  };

  /** A dual-threshold latch with hysterisis.
   *
   * See https://en.wikipedia.org/wiki/Schmitt_trigger
   */
  template<typename T>
  class SchmittTrigger
  {
  public:
    SchmittTrigger(T lowThreshold, T highThreshold, bool isInitiallyHigh)
    : d_lowThreshold(lowThreshold),
      d_highThreshold(highThreshold),
      d_isHigh(isInitiallyHigh)
    {
      if (lowThreshold >= highThreshold)
      {
        log::error("SchmittTrigger::SchmittTrigger") << "Low threshold must be lower than high threshold";
        throw std::runtime_error("Low threshold must be lower than high threshold");
      }
    }

    SchmittTriggerTransition next(T level)
    {
      if (d_isHigh && level <= d_lowThreshold)
      {
        d_isHigh = false;
        return SchmittTriggerTransition::Low;
      }
      else if (!d_isHigh && level >= d_highThreshold)
      {
        d_isHigh = true;
        return SchmittTriggerTransition::High;
      }

      return SchmittTriggerTransition::None;
    }

    bool isHigh() const { return d_isHigh; }
    T getLowThreshold() const { return d_lowThreshold; }
    T getHighThreshold() const { return d_highThreshold; }

  private:
    const T d_lowThreshold;
    const T d_highThreshold;
    bool d_isHigh;
  };
}
