#pragma once

#include <string>
#include <vector>
#include <memory>

namespace bold
{
  typedef unsigned char uchar;
  typedef unsigned short ushort;

  class MotionScript;

  /** Structure of the Robotis motion script files.
   *
   * Provides the ability to load a Robotis motion file and convert its contents
   * into the Bold Hearts script format.
   *
   * The file has 256 pages at 512 bytes per page, totalling 131,072 bytes.
   */
  class RobotisMotionFile
  {
  private:
    /// Page numbers run from 1 to 255
    static const uchar MAX_PAGE_ID = 255;

    struct Page
    {
      enum class MotionScriptPageSchedule : uchar
      {
        SPEED_BASE = 0,
        TIME_BASE = 0x0a
      };

      enum
      {
        INVALID_BIT_MASK    = 0x4000,
        TORQUE_OFF_BIT_MASK = 0x2000
      };

      static const uchar DEFAULT_SPEED = 32;
      static const uchar DEFAULT_ACCELERATION = 32;
      static const uchar DEFAULT_SLOPE = 0x55;
      static const MotionScriptPageSchedule DEFAULT_SCHEDULE = MotionScriptPageSchedule::TIME_BASE;

      bool isChecksumValid() const;
      uchar calculateChecksum() const;

      /// Steps numbered 0 to 6 (7 steps per page max)
      static const uchar MAXNUM_STEPS = 7;
      static const uchar MAXNUM_POSITIONS = 31;
      /// Page names may be no longer than 13 characters (14 including terminator)
      static const uchar MAXNUM_NAME = 13;

      inline uchar getPGain(uchar jointId) const { return (256 >> (slopes[jointId]>>4)) << 2; }
      void updateChecksum();
      void reset();

      //
      // Step Structure (total 64 bytes)
      //

      struct Step
      {
        ushort position[MAXNUM_POSITIONS]; // Joint position   0~61
        uchar pause;                       // Pause time       62
        uchar time;                        // Time             63
      };

      //
      // Page Structure (total 512 bytes)
      //

      // Fixed values across whole page:

      char name[MAXNUM_NAME+1];  // Name              0~13
      uchar reserved1;           // Reserved1         14
      uchar repeatCount;         // Repeat count      15
      uchar schedule;            // schedule          16
      uchar reserved2[3];        // reserved2         17~19
      uchar stepCount;           // Number of step    20
      uchar reserved3;           // reserved3         21
      uchar speed;               // Speed             22
      uchar reserved4;           // reserved4         23
      uchar accelerationTime;    // Acceleration time 24
      uchar nextPageIndex;       // Link to next      25
      uchar exit;                // Link to exit      26
      uchar reserved5[4];        // reserved5         27~30
      uchar checksum;            // checksum          31
      uchar slopes[31];          // CW/CCW slope      32~62
      uchar reserved6;           // reserved6         63

      // Data per-step

      Step steps[MAXNUM_STEPS];  // Page steps        64~511
    };

    Page d_pages[(int)MAX_PAGE_ID + 1];

  public:
    RobotisMotionFile(std::string const& filePath);

    /// Gets all populated pages that are not continuation targets of other pages.
    std::vector<uchar> getSequenceRootPageIndices() const;

    /// Writes DOT markup of a directed graph showing the relationship between pages within this file.
    void toDotText(std::ostream& out) const;

    std::shared_ptr<MotionScript> toMotionScript(uchar rootPageIndex);

    bool save(std::string const& filePath) const;

//     bool saveToJsonFile(uchar rootPageIndex, std::string const& filePath) const;
  };
}
