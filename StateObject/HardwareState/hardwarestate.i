%{
#include <StateObject/HardwareState/hardwarestate.hh>
%}

namespace bold
{
  class HardwareState : public StateObject
  {
  public:
    CM730Snapshot const& getCM730State() const;
    MX28Snapshot const& getMX28State(uchar jointId) const;

    unsigned long getReceivedBytes() const;
    unsigned long getTransmittedBytes() const;
  };
}
