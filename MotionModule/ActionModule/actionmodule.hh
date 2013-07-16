#pragma once

#include "../motionmodule.hh"

#include <stdio.h>
#include <string>
#include <set>

#include "../../Control/control.hh"

namespace bold
{
  class BodySection;

  class ActionModule : public MotionModule
  {
  public:
    enum
    {
      /// Valid page numbers range from 1 to 255.
      MAXNUM_PAGE = 256,
      /// Valid step of a page are numbered from 0 to 6 (7 steps per page max).
      MAXNUM_STEP = 7,
      /// Page names may be no longer than 13 characters.
      MAXNUM_NAME = 13
    };

    enum
    {
      SPEED_BASE_SCHEDULE = 0,
      TIME_BASE_SCHEDULE  = 0x0a
    };

    enum
    {
      INVALID_BIT_MASK    = 0x4000,
      TORQUE_OFF_BIT_MASK = 0x2000
    };

    // Page Header Structure (total 64 bytes)
    typedef struct
    {
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
    } PAGEHEADER;

    // Step Structure (total 64 bytes)
    typedef struct
    {
      unsigned short position[31];    // Joint position   0~61
      uchar pause;                    // Pause time       62
      uchar time;                     // Time             63
    } STEP;

    // Page Structure (total 512 bytes)
    typedef struct
    {
      PAGEHEADER header;              // Page header  0~63
      STEP step[MAXNUM_STEP];         // Page step    64~511
    } PAGE;

  private:
    FILE* d_file;
    PAGE d_playingPage;
    PAGE m_NextPlayPage;
    STEP m_CurrentStep;

    int d_playingPageIndex;
    /// Whether the next step will be the first of the action
    bool d_isFirstStepOfAction;
    int m_PageStepCount;
    bool d_isRunning;
    bool d_isStopRequested;
    bool d_playingFinished;

    bool d_active[21];
    uchar d_pGains[21];
    ushort d_values[21];

    std::vector<std::shared_ptr<Control const>> d_controls;

    bool isJointActive(uchar jointId) const { return d_active[jointId]; }

    bool verifyChecksum(PAGE *pPage);
    void setChecksum(PAGE *pPage);

  public:
    ActionModule(std::shared_ptr<MotionTaskScheduler> scheduler, std::string filename);

    ~ActionModule();

    void initialize() override;
    void step(std::shared_ptr<JointSelection> selectedJoints) override;
    void applyHead(std::shared_ptr<HeadSection> head) override;
    void applyArms(std::shared_ptr<ArmSection> arms) override;
    void applyLegs(std::shared_ptr<LegSection> legs) override;

    void applySection(std::shared_ptr<BodySection> section);

    std::vector<std::shared_ptr<Control const>> getControls() const { return d_controls; };

    /** Returns the set of page names found in the motion file.
     *
     * Not all pages have names, and there are several names that apply to
     * multiple pages. Playing an action by name results in the first page with
     * that name being started.
     */
    std::set<std::string> getPageNames();

    bool loadFile(std::string filename);
    bool createFile(std::string filename);
    bool start(int iPage);
    /** Play the first page having the specified name. */
    bool start(std::string namePage);
    bool start(int index, PAGE *pPage);
    /// Request the action to come to a stop soon.
    void stop();
    /** Stop immediately, irrespective of the current playing position. */
    void brake();
    bool isRunning();
    bool loadPage(int index, PAGE *pPage);
    bool savePage(int index, PAGE *pPage);
    void resetPage(PAGE *pPage);
  };
}
