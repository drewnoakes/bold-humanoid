#ifndef BOLD_LINE_RUN_TRACKER_HH
#define BOLD_LINE_RUN_TRACKER_HH

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
  * Hysterisis is used to address both noise and gaps at borders between the
  * two labels being tracked.
  */
  class LineRunTracker
  {
  public:
    ushort otherCoordinate;

    LineRunTracker(
      uchar inLabel,
      uchar onLabel,
      ushort otherCoordinate,
      uchar hysterisisLimit,
      std::function<void(ushort const, ushort const, ushort const)> callback
    )
    : inLabel(inLabel),
      onLabel(onLabel),
      otherCoordinate(otherCoordinate),
      callback(callback),
      hysterisisLimit(hysterisisLimit),
      hysterisis(0),
      state(State::Out)
    {}

    void reset()
    {
      state = State::Out;
    }

    void update(uchar label, ushort position)
    {
      switch (state)
      {
        case State::Out:
        {
          if (label == inLabel)
          {
            hysterisis = 0;
            state = State::In;
          }
          else
          {
            if (hysterisis != 0)
              hysterisis--;
          }
          break;
        }
        case State::In:
        {
          if (label == onLabel)
          {
            state = State::On;
            hysterisis = 0;
            startedAt = position;
          }
          else if (label == inLabel)
          {
            if (hysterisis != hysterisisLimit)
              hysterisis++;
          }
          else
          {
            if (hysterisis != 0)
              hysterisis--;
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
            hysterisis = 0;
            callback(startedAt, position, otherCoordinate);
          }
          else if (label == onLabel)
          {
            if (hysterisis != hysterisisLimit)
              hysterisis++;
          }
          else
          {
            if (hysterisis != 0)
              hysterisis--;
            else
              state = State::Out;
          }
          break;
        }
      }
    }

    uchar getHysterisisLimit() const { return hysterisisLimit; }
    void setHysterisisLimit(uchar limit) { hysterisisLimit = limit; }

  private:
    enum class State : uchar
    {
      // Not on the in-label, and not on the on-label after being on the in-label
      Out,
      // At the in-label (eg. green)
      In,
      // At the on-label, after being on the in-label (eg. white line)
      On
    };

    uchar inLabel; // eg: green
    uchar onLabel; // eg: white
    uchar hysterisisLimit;
    State state;
    ushort startedAt;
    std::function<void(ushort const, ushort const, ushort const)> callback;
    uint hysterisis;
  };
}

#endif