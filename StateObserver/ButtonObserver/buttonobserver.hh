#pragma once

#include "../typedstateobserver.hh"
#include "../../StateObject/HardwareState/hardwarestate.hh"

#include <mutex>

namespace bold
{
  class Agent;
  class HardwareState;

  enum class Button
  {
    Left = 1,
    Middle = 2
  };

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  /** Observes the state of a button (on the robot's rear panel) and reports upon presses.
   *
   * This is superior to reading CM730Snapshot directly, as it allows:
   *
   * - the button press to have a minimum duration (avoiding accidental pauses when falling backwards)
   *
   * - the press to be 'consumed' only once when a group of consumers share the tracker instance
   *   (avoiding infinite loops in an FSMOption, for example)
   *
   * Any unclaimed button presses are erased at the end of the think loop.
   */
  class ButtonTracker
  {
  public:
    ButtonTracker(Button button)
      : d_button(button),
        d_lastPressedState(false),
        d_isClaimed(false),
        d_downAt(0)
    {}

    Button getButton() const { return d_button; }

    void observe(bool isPressed);

    bool isPressedForMillis(double millis);

  private:
    Button d_button;
    std::mutex d_mutex;
    bool d_lastPressedState;
    bool d_isClaimed;
    Clock::Timestamp d_downAt;
  };

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  /** Observes HardwareState for button presses and notifies clients via ButtonTracker instances.
   */
  class ButtonObserver : public TypedStateObserver<HardwareState>
  {
  public:
    ButtonObserver();

    void observeTyped(std::shared_ptr<HardwareState const> const& hardwareState, SequentialTimer& timer) override;

    /** Gets a ButtonTracker instance that reports upon button presses. */
    std::shared_ptr<ButtonTracker> track(Button button);

  private:
    std::vector<std::shared_ptr<ButtonTracker>> d_trackers;
  };
}
