%{
#include <StateObject/HardwareState/hardwarestate.hh>
%}

namespace bold
{
  class HardwareState : public StateObject
  {
  public:
    std::unique_ptr<CM730Snapshot const> const& getCM730State() const;
    std::unique_ptr<MX28Snapshot const> const& getMX28State(uchar jointId) const;

    unsigned long getReceivedBytes() const;
    unsigned long getTransmittedBytes() const;
  };
}
