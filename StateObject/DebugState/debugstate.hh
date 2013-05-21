#pragma once

#include "../stateobject.hh"
#include "../../Colour/colour.hh"

#include <iostream>
#include <memory>
#include <vector>

namespace bold
{
  class DebugState : public StateObject
  {
  public:
    DebugState(unsigned gameControllerMessageCount, unsigned ignoredMessageCount,
               Colour::bgr eyeColour, Colour::bgr headColour,
               bool redLed, bool greenLed, bool blueLed)
    : d_gameControllerMessageCount(gameControllerMessageCount),
      d_ignoredMessageCount(ignoredMessageCount),
      d_eyeColour(eyeColour),
      d_headColour(headColour),
      d_redLed(redLed),
      d_greenLed(greenLed),
      d_blueLed(blueLed)
    {}

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    unsigned d_gameControllerMessageCount;
    unsigned d_ignoredMessageCount;
    Colour::bgr d_eyeColour;
    Colour::bgr d_headColour;
    bool d_redLed;
    bool d_greenLed;
    bool d_blueLed;
  };
}
