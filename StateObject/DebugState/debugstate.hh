#pragma once

#include "../stateobject.hh"
#include "../../BodyControl/bodycontrol.hh"
#include "../../Colour/colour.hh"

#include <memory>
#include <vector>

namespace bold
{
  class DebugState : public StateObject
  {
  public:
    DebugState(unsigned gameControllerMessageCount, unsigned ignoredMessageCount,
               unsigned sentTeamMessageCount, unsigned receivedTeamMessageCount,
               std::shared_ptr<DebugControl> debugControl)
    : d_gameControllerMessageCount(gameControllerMessageCount),
      d_ignoredMessageCount(ignoredMessageCount),
      d_sentTeamMessageCount(sentTeamMessageCount),
      d_receivedTeamMessageCount(receivedTeamMessageCount),
      d_eyeColour(debugControl->getEyeColour()),
      d_foreheadColour(debugControl->getForeheadColour()),
      d_redLed(debugControl->isRedPanelLedLit()),
      d_greenLed(debugControl->isGreenPanelLedLit()),
      d_blueLed(debugControl->isBluePanelLedLit())
    {}

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    unsigned d_gameControllerMessageCount;
    unsigned d_ignoredMessageCount;
    unsigned d_sentTeamMessageCount;
    unsigned d_receivedTeamMessageCount;
    Colour::bgr d_eyeColour;
    Colour::bgr d_foreheadColour;
    bool d_redLed;
    bool d_greenLed;
    bool d_blueLed;
  };
}
