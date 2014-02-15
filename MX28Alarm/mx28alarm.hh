#pragma once

#include <sstream>
#include <cassert>
#include <vector>

namespace bold
{
  typedef unsigned char uchar;

  /** Models the alarm state of an MX28.
  *
  * Bits are assigned as follows:
  *
  * bit 0 - input voltage error - applied voltage is outside MX28::P_LOW_LIMIT_VOLTAGE and MX28::P_HIGH_LIMIT_VOLTAGE
  * bit 1 - angle limit error - goal position is outside MX28::P_CW_ANGLE_LIMIT_L and MX28::P_CCW_ANGLE_LIMIT_L
  * bit 2 - overheating error - internal temperature exceeds the limit set via MX28::P_HIGH_LIMIT_TEMPERATURE
  * bit 3 - range error - command is given beyond range of usage
  * bit 4 - checksum error - the checksum of the received instruction packet is incorrect
  * bit 5 - overoad error - the current load cannot be controlled with the set maximum torque
  * bit 6 - instruction error - undefined instruction, or action command delivered without reg_write
  * bit 7 - unused?
  */
  class MX28Alarm
  {
  public:
    const static uchar MAXBIT = 6;

    MX28Alarm()
    {}

    MX28Alarm(uchar flags)
    : d_flags (flags)
    {}

    MX28Alarm& operator=(const uchar flags)
    {
      d_flags = flags;
      return *this;
    }

    uchar getFlags() const { return d_flags; }

    bool isSet(uchar bitIndex) const
    {
      assert(bitIndex <= MAXBIT);

      return ((d_flags >> bitIndex) & 1) == 1;
    }

    bool hasError() const { return d_flags != 0; }

    bool hasInputVoltageError() const { return isSet(0); }
    bool hasAngleLimitError() const { return isSet(1); }
    bool hasOverheatedError() const { return isSet(2); }
    bool hasRangeError() const { return isSet(3); }
    bool hasChecksumError() const { return isSet(4); }
    bool hasOverloadError() const { return isSet(5); }
    bool hasInstructionError() const { return isSet(6); }

    bool operator==(MX28Alarm const& other) const
    {
      return d_flags == other.d_flags;
    }

    bool operator!=(MX28Alarm const& other) const
    {
      return d_flags != other.d_flags;
    }

    std::vector<int> diff(MX28Alarm const& other) const
    {
      std::vector<int> diffFlags;
      for (uchar i = 0; i <= MAXBIT; i++)
      {
        if (isSet(i) != other.isSet(i))
          diffFlags.push_back(i);
      }
      return diffFlags;
    }

    std::vector<std::string> getSetNames() const
    {
      std::vector<std::string> names;

      for (uchar i = 0; i <= MAXBIT; i++)
      {
        if (isSet(i))
          names.push_back(getName(i));
      }

      return names;
    }

    std::string toString() const
    {
      std::stringstream stream;
      stream << *this;
      return stream.str();
    }

    static std::string getName(uchar bitIndex)
    {
      assert(bitIndex <= MAXBIT);

      static std::vector<std::string> names = {
        "Input Voltage Limit Breached",
        "Angle Limit Breached",
        "Overheated",
        "Out Of Range",
        "Checksum Error",
        "Overloaded",
        "Instruction Error"
      };
      return names[bitIndex];
    }

    friend std::ostream& operator<<(std::ostream& stream, MX28Alarm const& mx28Alarm)
    {
      bool first = true;

      for (uchar i = 0; i <= MAXBIT; i++)
      {
        if (mx28Alarm.isSet(i))
        {
          if (!first)
            stream << ", ";
          else
            first = false;

          stream << getName(i);
        }
      }

      return stream;
    }

  private:
    uchar d_flags;
  };
}
