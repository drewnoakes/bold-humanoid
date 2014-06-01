#pragma once

#include <memory>
#include <vector>

#include "../stateobject.hh"
#include "../../CM730Snapshot/cm730snapshot.hh"
#include "../../JointId/jointid.hh"
#include "../../MX28Snapshot/mx28snapshot.hh"
#include "../../util/assert.hh"

namespace bold
{
  typedef unsigned char uchar;
  typedef unsigned long ulong;

  class HardwareState : public StateObject
  {
  public:
    HardwareState(std::unique_ptr<CM730Snapshot const> cm730State,
                  std::vector<std::unique_ptr<MX28Snapshot const>> mx28States,
                  ulong rxBytes,
                  ulong txBytes,
                  ulong motionCycleNumber)
    : d_cm730State(std::move(cm730State)),
      d_mx28States(std::move(mx28States)),
      d_rxBytes(rxBytes),
      d_txBytes(txBytes),
      d_motionCycleNumber(motionCycleNumber)
    {
      ASSERT(d_mx28States.size() == 20);
    }

    CM730Snapshot const& getCM730State() const
    {
      return *d_cm730State;
    }

    MX28Snapshot const& getMX28State(uchar jointId) const
    {
      ASSERT(jointId >= (uchar)JointId::MIN && jointId <= (uchar)JointId::MAX);
      ASSERT(d_mx28States.size() >= jointId);

      return *d_mx28States[jointId - 1];
    }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

    ulong getReceivedBytes() const { return d_rxBytes; }
    ulong getTransmittedBytes() const { return d_txBytes; }
    ulong getMotionCycleNumber() const { return d_motionCycleNumber; }

  private:
    std::unique_ptr<CM730Snapshot const> d_cm730State;
    std::vector<std::unique_ptr<MX28Snapshot const>> d_mx28States;
    ulong d_rxBytes;
    ulong d_txBytes;
    ulong d_motionCycleNumber;
  };
}
