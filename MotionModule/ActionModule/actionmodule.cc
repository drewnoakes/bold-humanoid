#include "actionmodule.hh"

#include "../../AgentState/agentstate.hh"
#include "../../BodyControl/bodycontrol.hh"
#include "../../MotionScriptFile/motionscriptfile.hh"
#include "../../MotionScriptPage/motionscriptpage.hh"
#include "../../MotionTaskScheduler/motiontaskscheduler.hh"
#include "../../MX28Snapshot/mx28snapshot.hh"
#include "../../StateObject/HardwareState/hardwarestate.hh"
#include "../../ThreadId/threadid.hh"

#include <cassert>
#include <iostream>
#include <string.h>

using namespace bold;
using namespace std;

ActionModule::ActionModule(std::shared_ptr<MotionTaskScheduler> scheduler, std::shared_ptr<MotionScriptFile> file)
: MotionModule("action", scheduler),
  d_file(file),
  d_isRunning(false)
{
  assert(file);
  
  for (string name : file->getPageNames())
    d_controls.push_back(Control::createAction(name, [this,name]() { start(name); }));
}

void ActionModule::initialize()
{
  d_isRunning = false;
}

bool ActionModule::isRunning()
{
  return d_isRunning;
}

void ActionModule::step(shared_ptr<JointSelection> selectedJoints)
{
  assert(ThreadId::isMotionLoopThread());

  ///////////////// Static
  const int JOINT_ARRAY_LENGTH = 22;
  static ushort wpStartAngle1024[JOINT_ARRAY_LENGTH];
  static ushort wpTargetAngle1024[JOINT_ARRAY_LENGTH];
  static short ipMovingAngle1024[JOINT_ARRAY_LENGTH];
  static short ipMainAngle1024[JOINT_ARRAY_LENGTH];
  static short ipAccelAngle1024[JOINT_ARRAY_LENGTH];
  static short ipMainSpeed1024[JOINT_ARRAY_LENGTH];
  static short ipLastOutSpeed1024[JOINT_ARRAY_LENGTH];
  static short ipGoalSpeed1024[JOINT_ARRAY_LENGTH];
  static uchar bpFinishType[JOINT_ARRAY_LENGTH];
  static ushort wUnitTimeCount;
  static ushort wUnitTimeNum;
  static ushort wPauseTime;
  static ushort wUnitTimeTotalNum;
  static ushort wAccelStep;
  static uchar bSection;
  static uchar bPlayRepeatCount;
  static ushort wNextPlayPage;

  auto hw = AgentState::get<HardwareState>();

  /////////////// Enum

  /**************************************
  * Section             /----\
  *                    /|    |\
  *        /+---------/ |    | \
  *       / |        |  |    |  \
  * -----/  |        |  |    |   \----
  *      PRE  MAIN   PRE MAIN POST PAUSE
  ***************************************/
  enum{ PRE_SECTION, MAIN_SECTION, POST_SECTION, PAUSE_SECTION };
  enum{ ZERO_FINISH, NON_ZERO_FINISH};

  if (d_isFirstStepOfAction)
  {
    // Special treatment for the first step of a new action
    d_isRunning = true;
    d_isFirstStepOfAction = false;
    d_playingFinished = false;
    d_isStopRequested = false;
    wUnitTimeCount = 0;
    wUnitTimeNum = 0;
    wPauseTime = 0;
    bSection = PAUSE_SECTION;
    m_PageStepCount = 0;
    bPlayRepeatCount = d_playingPage->getRepeatCount();
    wNextPlayPage = 0;

    for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    {
        // Only update selected joints
      if (!(*selectedJoints)[jointId])
        continue;

      wpTargetAngle1024[jointId] = hw->getMX28State(jointId)->presentPositionValue;
      ipLastOutSpeed1024[jointId] = 0;
      ipMovingAngle1024[jointId] = 0;
      ipGoalSpeed1024[jointId] = 0;
    }
  }

  if (!d_isRunning)
  {
    setCompletedFlag();
    return;
  }

  if (wUnitTimeCount < wUnitTimeNum)
  {
    wUnitTimeCount++;
    if (bSection != PAUSE_SECTION)
    {
      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      {
        // Only update selected joints
        if (!(*selectedJoints)[jointId])
          continue;

        if (ipMovingAngle1024[jointId] == 0)
        {
          d_values[jointId] = wpStartAngle1024[jointId];
        }
        else if (bSection == PRE_SECTION)
        {
          short iSpeedN = (short)(((long)(ipMainSpeed1024[jointId] - ipLastOutSpeed1024[jointId]) * wUnitTimeCount) / wUnitTimeNum);
          ipGoalSpeed1024[jointId] = ipLastOutSpeed1024[jointId] + iSpeedN;
          ipAccelAngle1024[jointId] =  (short)((((long)(ipLastOutSpeed1024[jointId] + (iSpeedN >> 1)) * wUnitTimeCount * 144) / 15) >> 9);

          d_values[jointId] = wpStartAngle1024[jointId] + ipAccelAngle1024[jointId];
        }
        else if (bSection == MAIN_SECTION)
        {
          d_values[jointId] = wpStartAngle1024[jointId] + (short)(((long)(ipMainAngle1024[jointId])*wUnitTimeCount) / wUnitTimeNum);
          ipGoalSpeed1024[jointId] = ipMainSpeed1024[jointId];
        }
        else
        {
          assert(bSection == POST_SECTION);
          if (wUnitTimeCount == (wUnitTimeNum-1))
          {
            d_values[jointId] = wpTargetAngle1024[jointId];
          }
          else
          {
            if (bpFinishType[jointId] == ZERO_FINISH)
            {
              short iSpeedN = (short)(((long)(0 - ipLastOutSpeed1024[jointId]) * wUnitTimeCount) / wUnitTimeNum);
              ipGoalSpeed1024[jointId] = ipLastOutSpeed1024[jointId] + iSpeedN;
              d_values[jointId] = wpStartAngle1024[jointId] + (short)((((long)(ipLastOutSpeed1024[jointId] + (iSpeedN>>1)) * wUnitTimeCount * 144) / 15) >> 9);
            }
            else // NON_ZERO_FINISH
            {
              // MAIN Section
              d_values[jointId] = wpStartAngle1024[jointId] + (short)(((long)(ipMainAngle1024[jointId]) * wUnitTimeCount) / wUnitTimeNum);
              ipGoalSpeed1024[jointId] = ipMainSpeed1024[jointId];
            }
          }
        }

        d_pGains[jointId] = d_playingPage->getPGain(jointId);
//         d_pGains[jointId] = (256 >> (d_playingPage->getSlope(jointId)>>4)) << 2;
      }
    }
  }
  else if (wUnitTimeCount >= wUnitTimeNum) // Section
  {
    wUnitTimeCount = 0;

    for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    {
      if ((*selectedJoints)[jointId])
      {
        wpStartAngle1024[jointId] = d_values[jointId];
        ipLastOutSpeed1024[jointId] = ipGoalSpeed1024[jointId];
      }
    }

    // Section (PRE -> MAIN -> POST -> (PAUSE or PRE) ...)
    switch (bSection)
    {
      case PRE_SECTION:
      {
        // MAIN Section
        bSection = MAIN_SECTION;
        wUnitTimeNum =  wUnitTimeTotalNum - (wAccelStep << 1);

        for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
        {
          if ((*selectedJoints)[jointId])
          {
            if (bpFinishType[jointId] == NON_ZERO_FINISH)
            {
              ipMainAngle1024[jointId] = (wUnitTimeTotalNum - wAccelStep) == 0
                ? ipMainAngle1024[jointId] = 0
                : ipMainAngle1024[jointId] = (short)((((long)(ipMovingAngle1024[jointId] - ipAccelAngle1024[jointId])) * wUnitTimeNum) / (wUnitTimeTotalNum - wAccelStep));
            }
            else
            {
              assert(bpFinishType[jointId] == ZERO_FINISH);
              ipMainAngle1024[jointId] = ipMovingAngle1024[jointId] - ipAccelAngle1024[jointId] - (short)((((long)ipMainSpeed1024[jointId] * wAccelStep * 12) / 5) >> 8);
            }
          }
        }
        break;
      }
      case MAIN_SECTION:
      {
        // POST Section
        bSection = POST_SECTION;
        wUnitTimeNum = wAccelStep;

        for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
        {
          if ((*selectedJoints)[jointId])
            ipMainAngle1024[jointId] = ipMovingAngle1024[jointId] - ipMainAngle1024[jointId] - ipAccelAngle1024[jointId];
        }
        break;
      }
      case POST_SECTION:
      {
        // Pause time
        if (wPauseTime)
        {
          bSection = PAUSE_SECTION;
          wUnitTimeNum = wPauseTime;
        }
        else
        {
          bSection = PRE_SECTION;
        }
        break;
      }
      case PAUSE_SECTION:
      {
        // PRE Section
        bSection = PRE_SECTION;
        memset(ipLastOutSpeed1024, 0, sizeof(ipLastOutSpeed1024));
        break;
      }
    }

    // PRE Section
    if (bSection == PRE_SECTION)
    {
      if (d_playingFinished)
      {
        d_isRunning = false;
        setCompletedFlag();
        return;
      }

      m_PageStepCount++;

      if (m_PageStepCount > d_playingPage->getStepCount())
      {
        d_playingPage = m_NextPlayPage;
        if (d_playingPageIndex != wNextPlayPage)
          bPlayRepeatCount = d_playingPage->getRepeatCount();
        m_PageStepCount = 1;
        d_playingPageIndex = wNextPlayPage;
      }

      if (m_PageStepCount == d_playingPage->getStepCount())
      {
        if (d_isStopRequested)
        {
          wNextPlayPage = d_playingPage->getExit();
        }
        else
        {
          bPlayRepeatCount--;
          wNextPlayPage = bPlayRepeatCount > 0
            ? d_playingPageIndex
            : d_playingPage->getNext();
        }

        if (wNextPlayPage == 0)
        {
          d_playingFinished = true;
        }
        else
        {
          if (d_playingPageIndex != wNextPlayPage)
            m_NextPlayPage = d_file->getPageByIndex(wNextPlayPage);
          else
            m_NextPlayPage = d_playingPage;

          if (m_NextPlayPage->getRepeatCount() == 0 || m_NextPlayPage->getStepCount() == 0)
            d_playingFinished = true;
        }
      }

      //////// Step
      wPauseTime = (((ushort)d_playingPage->getStepPause(m_PageStepCount-1)) << 5) / d_playingPage->getSpeed();
      ushort wMaxSpeed256 = ((ushort)d_playingPage->getStepTime(m_PageStepCount-1) * (ushort)d_playingPage->getSpeed()) >> 5;
      if (wMaxSpeed256 == 0)
        wMaxSpeed256 = 1;
      ushort wMaxAngle1024 = 0;

      ////////// Joint
      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      {
        if (!(*selectedJoints)[jointId])
          continue;

        ipAccelAngle1024[jointId] = 0;

        // Find current target angle
        ushort wCurrentTargetAngle = d_playingPage->getStepPosition(m_PageStepCount-1, jointId) & MotionScriptPage::INVALID_BIT_MASK
          ? wpTargetAngle1024[jointId]
          : d_playingPage->getStepPosition(m_PageStepCount-1, jointId);

        // Update start, prev_target, curr_target
        wpStartAngle1024[jointId] = wpTargetAngle1024[jointId];
        ushort wPrevTargetAngle = wpTargetAngle1024[jointId];
        wpTargetAngle1024[jointId] = wCurrentTargetAngle;

        // Find Moving offset
        ipMovingAngle1024[jointId] = (int)(wpTargetAngle1024[jointId] - wpStartAngle1024[jointId]);

        // Find Next target angle
        ushort wNextTargetAngle;
        if (m_PageStepCount == d_playingPage->getStepCount())
        {
          wNextTargetAngle = d_playingFinished
            ? wCurrentTargetAngle
            : m_NextPlayPage->getStepPosition(0, jointId) & MotionScriptPage::INVALID_BIT_MASK
              ? wCurrentTargetAngle
              : m_NextPlayPage->getStepPosition(0, jointId);
        }
        else
        {
          wNextTargetAngle = d_playingPage->getStepPosition(m_PageStepCount, jointId) & MotionScriptPage::INVALID_BIT_MASK
            ? wCurrentTargetAngle
            : d_playingPage->getStepPosition(m_PageStepCount, jointId);
        }

        // Find direction change
        bool bDirectionChanged;
        if (((wPrevTargetAngle < wCurrentTargetAngle) && (wCurrentTargetAngle < wNextTargetAngle))
         || ((wPrevTargetAngle > wCurrentTargetAngle) && (wCurrentTargetAngle > wNextTargetAngle)))
        {
          bDirectionChanged = false;
        }
        else
        {
          bDirectionChanged = true;
        }

        // Find finish type
        bpFinishType[jointId] = bDirectionChanged || wPauseTime || d_playingFinished
          ? ZERO_FINISH
          : NON_ZERO_FINISH;

        if (d_playingPage->getSchedule() == MotionScriptPageSchedule::SPEED_BASE)
        {
          // MaxAngle1024 update
          ushort wTmp = ipMovingAngle1024[jointId] < 0
            ? -ipMovingAngle1024[jointId]
            :  ipMovingAngle1024[jointId];

          if (wTmp > wMaxAngle1024)
            wMaxAngle1024 = wTmp;
        }
      }

      //wUnitTimeNum = ((wMaxAngle1024*300/1024) /(wMaxSpeed256 * 720/256)) /7.8msec;
      //             = ((128*wMaxAngle1024*300/1024) /(wMaxSpeed256 * 720/256)) ;    (/7.8msec == *128)
      //             = (wMaxAngle1024*40) /(wMaxSpeed256 *3);
      wUnitTimeTotalNum = d_playingPage->getSchedule() == MotionScriptPageSchedule::TIME_BASE
        ? wMaxSpeed256 //TIME BASE 051025
        : (wMaxAngle1024 * 40) / (wMaxSpeed256 * 3);

      wAccelStep = d_playingPage->getAcceleration();
      if (wUnitTimeTotalNum <= (wAccelStep << 1))
      {
        if (wUnitTimeTotalNum == 0)
        {
          wAccelStep = 0;
        }
        else
        {
          wAccelStep = (wUnitTimeTotalNum - 1) >> 1;
          if (wAccelStep == 0)
            wUnitTimeTotalNum = 0;
        }
      }

      ulong ulTotalTime256T = ((ulong)wUnitTimeTotalNum) << 1;// /128 * 256
      ulong ulPreSectionTime256T = ((ulong)wAccelStep) << 1;// /128 * 256
      ulong ulMainTime256T = ulTotalTime256T - ulPreSectionTime256T;
      long lDivider1 = ulPreSectionTime256T + (ulMainTime256T << 1);
      long lDivider2 = (ulMainTime256T << 1);

      if (lDivider1 == 0)
        lDivider1 = 1;

      if (lDivider2 == 0)
        lDivider2 = 1;

      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      {
        if ((*selectedJoints)[jointId])
        {
          long lStartSpeed1024_PreTime_256T = (long)ipLastOutSpeed1024[jointId] * ulPreSectionTime256T; //  *300/1024 * 1024/720 * 256 * 2
          long lMovingAngle_Speed1024Scale_256T_2T = (((long)ipMovingAngle1024[jointId]) * 2560L) / 12;

          ipMainSpeed1024[jointId] = bpFinishType[jointId] == ZERO_FINISH
            ? (short)((lMovingAngle_Speed1024Scale_256T_2T - lStartSpeed1024_PreTime_256T) / lDivider2)
            : (short)((lMovingAngle_Speed1024Scale_256T_2T - lStartSpeed1024_PreTime_256T) / lDivider1);

          ipMainSpeed1024[jointId] = Math::clamp(ipMainSpeed1024[jointId], (short)-1023, (short)1023);
        }
      }

      wUnitTimeNum = wAccelStep;
    } // PreSection
  }
}

void ActionModule::applySection(shared_ptr<BodySection> section)
{
  section->visitJoints([&section,this](shared_ptr<JointControl> joint)
  {
    joint->setValue(d_values[joint->getId()]);
    joint->setPGain(d_pGains[joint->getId()]);
  });
}

void ActionModule::applyHead(shared_ptr<HeadSection> head) { applySection(dynamic_pointer_cast<BodySection>(head)); }
void ActionModule::applyArms(shared_ptr<ArmSection> arms) { applySection(dynamic_pointer_cast<BodySection>(arms)); }
void ActionModule::applyLegs(shared_ptr<LegSection> legs) { applySection(dynamic_pointer_cast<BodySection>(legs)); }

bool ActionModule::start(int pageIndex)
{
  auto page = d_file->getPageByIndex(pageIndex);

  return page ? start(pageIndex, page) : false;
}

bool ActionModule::start(string const& pageName)
{
  for (int index = 0; index < MotionScriptFile::MAX_PAGE_ID; index++)
  {
    auto page = d_file->getPageByIndex(index);
    if (page->getName() == pageName)
      return start(index, page);
  }

  cerr << "[ActionModule::start] No page with name " << pageName << " found" << endl;
  return false;
}

bool ActionModule::start(int index, shared_ptr<MotionScriptPage> page)
{
  if (d_isRunning)
  {
    cerr << "[ActionModule::start] Ignoring request to play page " << index << " -- already playing page " << d_playingPageIndex << endl;
    return false;
  }

  d_playingPage = page;

  if (d_playingPage->getRepeatCount() == 0 || d_playingPage->getStepCount() == 0)
  {
    cerr << "[ActionModule::start] Page index " << index << " has no steps to perform" << endl;
    return false;
  }

  d_playingPageIndex = index;
  d_isFirstStepOfAction = true;

  d_isRunning = false; // will be set to true once 'step' is called

  // copy the current pose somehow at this time?
  auto hw = AgentState::get<HardwareState>();
  assert(hw);
  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    d_values[jointId] = hw->getMX28State(jointId)->presentPositionValue;

  getScheduler()->add(this,
                      Priority::Optional,  true,  // HEAD   Interuptable::YES
                      Priority::Important, true,  // ARMS
                      Priority::Important, true); // LEGS

  return true;
}

void ActionModule::stop()
{
  d_isStopRequested = true;
}

void ActionModule::brake()
{
  d_isRunning = false;
}
