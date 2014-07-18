#pragma once

#include <opencv2/core/core.hpp>
#include <functional>

namespace bold
{
  /**
  * A state machine that calls back when it detects a run of one label sandwiched
  * between runs of another label.
  *
  * This type was designed for finding line segments in an image row/column.
  *
  * Hysteresis is used to address both noise and gaps at borders between the
  * two labels being tracked.
  */
  class LineRunTracker
  {
  public:
    LineRunTracker(
      uint8_t inLabel,
      uint8_t onLabel,
      ushort otherCoordinate,
      uint8_t hysteresisLimit,
      std::function<void(ushort const, ushort const, ushort const)> callback
    )
      : otherCoordinate(otherCoordinate),
	inLabel(inLabel),
	onLabel(onLabel),
	hysteresisLimit(hysteresisLimit),
	state(State::Out),
	startedAt(0),
	callback(callback),
	hysteresis(0)
    {}

    void reset()
    {
      state = State::Out;
    }

    void update(uint8_t label, ushort position);

    uint8_t getHysteresisLimit() const { return hysteresisLimit; }
    void setHysteresisLimit(uint8_t limit) { hysteresisLimit = limit; }

    ushort otherCoordinate;

  private:
    enum class State : uint8_t
    {
      // Not on the in-label, and not on the on-label after being on the in-label
      Out,
      // At the in-label (eg. green)
      In,
      // At the on-label, after being on the in-label (eg. white line)
      On
    };

    uint8_t inLabel; // eg: green
    uint8_t onLabel; // eg: white
    uint8_t hysteresisLimit;
    State state;
    ushort startedAt;
    std::function<void(ushort const, ushort const, ushort const)> callback;
    uint hysteresis;
  };

  inline void LineRunTracker::update(uint8_t label, ushort position)
  {
    switch (state)
    {
    case State::Out:
    {
      if (label == inLabel)
      {
	hysteresis = 0;
	state = State::In;
      }
      else
      {
	if (hysteresis != 0)
	  hysteresis--;
      }
      break;
    }
    case State::In:
    {
      if (label == onLabel)
      {
	state = State::On;
	hysteresis = 0;
	startedAt = position;
      }
      else if (label == inLabel)
      {
	if (hysteresis != hysteresisLimit)
	  hysteresis++;
      }
      else
      {
	if (hysteresis != 0)
	  hysteresis--;
	else
	  state = State::Out;
      }
      break;
    }
    case State::On:
    {
      if (label == inLabel)
      {
	// we completed a run!
	state = State::In;
	hysteresis = 0;
	callback(startedAt, position, otherCoordinate);
      }
      else if (label == onLabel)
      {
	if (hysteresis != hysteresisLimit)
	  hysteresis++;
      }
      else
      {
	if (hysteresis != 0)
	  hysteresis--;
	else
	  state = State::Out;
      }
      break;
    }
    }
  }
}
