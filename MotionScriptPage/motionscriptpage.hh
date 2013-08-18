#pragma once

#include <string>
#include <stdexcept>
#include <string.h>

namespace bold
{
  class MotionScriptFile;

  typedef unsigned char uchar;
  typedef unsigned short ushort;

  enum class MotionScriptPageSchedule : uchar
  {
    SPEED_BASE = 0,
    TIME_BASE = 0x0a
  };

  class MotionScriptPage
  {
    friend class MotionScriptFile;

  public:
    static const uchar DEFAULT_SPEED = 32;
    static const uchar DEFAULT_ACCELERATION = 32;
    static const uchar DEFAULT_SLOPE = 0x55;
    static const MotionScriptPageSchedule DEFAULT_SCHEDULE = MotionScriptPageSchedule::TIME_BASE;

    enum
    {
      INVALID_BIT_MASK    = 0x4000,
      TORQUE_OFF_BIT_MASK = 0x2000
    };

  private:
    /// Steps numbered 0 to 6 (7 steps per page max)
    static const uchar MAXNUM_STEPS = 7;
    static const uchar MAXNUM_POSITIONS = 31;
    /// Page names may be no longer than 13 characters (14 including terminator)
    static const uchar MAXNUM_NAME = 13;

    // Step Structure (total 64 bytes)
    struct Step
    {
      ushort position[MAXNUM_POSITIONS]; // Joint position   0~61
      uchar pause;                       // Pause time       62
      uchar time;                        // Time             63
    };

  public:
    /// Resets the page, clearing all motion instructions from it.
    void reset();

    bool isChecksumValid() const;

    bool isEmpty() const;

    inline std::string getName() const { return std::string(reinterpret_cast<char const*>(name)); }
    inline void setName(std::string name)
    {
      if (name.size() > MAXNUM_NAME)
        throw std::runtime_error("Name is too long");

      memset(this->name, MAXNUM_NAME+1, 0);
      memcpy(this->name, name.c_str(), name.size());
    }

    inline uchar getRepeatCount() const { return repeat; }
    inline void setRepeatCount(uchar count) { repeat = count; }

    inline uchar getStepCount() const { return stepnum; }
//    inline void setStepCount(uchar count) { stepnum = count; }

    inline uchar getSlope(uchar jointId) const { return slope[jointId]; }
    inline uchar getPGain(uchar jointId) const { return (256 >> (slope[jointId]>>4)) << 2; }

    inline uchar getNext() const { return next; }
    inline uchar getSpeed() const { return speed; }

    inline MotionScriptPageSchedule getSchedule() const { return schedule ? MotionScriptPageSchedule::TIME_BASE : MotionScriptPageSchedule::SPEED_BASE; }
    inline uchar getAcceleration() const { return accel; }

    inline uchar getStepPause(uchar stepIndex) const { return steps[stepIndex].pause; }
    inline uchar getStepTime(uchar stepIndex) const { return steps[stepIndex].time; }
    inline ushort getStepPosition(uchar stepIndex, uchar jointId) const { return steps[stepIndex].position[jointId]; }

  protected:
    void updateChecksum();
    uchar calculateChecksum() const;

  private:
    //
    // Page Structure (total 512 bytes)
    //

    // Fixed values across whole page:

    uchar name[MAXNUM_NAME+1]; // Name              0~13
    uchar reserved1;           // Reserved1         14
    uchar repeat;              // Repeat count      15
    uchar schedule;            // schedule          16
    uchar reserved2[3];        // reserved2         17~19
    uchar stepnum;             // Number of step    20
    uchar reserved3;           // reserved3         21
    uchar speed;               // Speed             22
    uchar reserved4;           // reserved4         23
    uchar accel;               // Acceleration time 24
    uchar next;                // Link to next      25
    uchar exit;                // Link to exit      26
    uchar reserved5[4];        // reserved5         27~30
    uchar checksum;            // checksum          31
    uchar slope[31];           // CW/CCW slope      32~62
    uchar reserved6;           // reserved6         63

    // Data per-step

    Step steps[MAXNUM_STEPS];  // Page steps        64~511
  };
}
