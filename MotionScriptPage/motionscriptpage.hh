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

    inline std::string getName() const { return std::string(reinterpret_cast<char const*>(d_name)); }
//     inline void setName(std::string name)
//     {
//       if (name.size() > MAXNUM_NAME)
//         throw std::runtime_error("Name is too long");
//
//       memset(this->d_name, MAXNUM_NAME+1, 0);
//       memcpy(this->d_name, name.c_str(), name.size());
//     }

    inline uchar getRepeatCount() const { return d_repeatCount; }
//    inline void setRepeatCount(uchar count) { d_repeatCount = count; }

    inline uchar getStepCount() const { return d_stepCount; }
//    inline void setStepCount(uchar count) { stepnum = count; }

    inline uchar getSlope(uchar jointId) const { return d_slopes[jointId]; }
    inline uchar getPGain(uchar jointId) const { return (256 >> (d_slopes[jointId]>>4)) << 2; }

    inline uchar getNextPageIndex() const { return d_nextPageIndex; }
    inline uchar getSpeed() const { return d_speed; }

    inline MotionScriptPageSchedule getSchedule() const { return d_schedule ? MotionScriptPageSchedule::TIME_BASE : MotionScriptPageSchedule::SPEED_BASE; }
    inline uchar getAccelerationTime() const { return d_accelerationTime; }

    inline uchar getStepPause(uchar stepIndex) const { return d_steps[stepIndex].pause; }
    inline uchar getStepTime(uchar stepIndex) const { return d_steps[stepIndex].time; }
    inline ushort getStepPosition(uchar stepIndex, uchar jointId) const { return d_steps[stepIndex].position[jointId]; }

  protected:
    void updateChecksum();
    uchar calculateChecksum() const;

  private:
    //
    // Page Structure (total 512 bytes)
    //

    // Fixed values across whole page:

    uchar d_name[MAXNUM_NAME+1]; // Name              0~13
    uchar reserved1;             // Reserved1         14
    uchar d_repeatCount;         // Repeat count      15
    uchar d_schedule;            // schedule          16
    uchar reserved2[3];          // reserved2         17~19
    uchar d_stepCount;           // Number of step    20
    uchar reserved3;             // reserved3         21
    uchar d_speed;               // Speed             22
    uchar reserved4;             // reserved4         23
    uchar d_accelerationTime;    // Acceleration time 24
    uchar d_nextPageIndex;       // Link to next      25
    uchar exit;                  // Link to exit      26
    uchar reserved5[4];          // reserved5         27~30
    uchar d_checksum;            // checksum          31
    uchar d_slopes[31];          // CW/CCW slope      32~62
    uchar reserved6;             // reserved6         63

    // Data per-step

    Step d_steps[MAXNUM_STEPS];  // Page steps        64~511
  };
}
