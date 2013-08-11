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

  for (shared_ptr<MotionScriptPage> page : file->getSequenceRoots())
  {
    int pageIndex = file->indexOf(page);
    stringstream label;
    label << page->getName() << " (" << pageIndex << ")";
    cout << "[ActionModule::ActionModule] Found root page: " << label.str() << endl;
    d_controls.push_back(Control::createAction(label.str(), [this,pageIndex]() { start(pageIndex); }));
  }
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

  if (!d_isRunning)
  {
    setCompletedFlag();
    return;
  }

  /////////////// Enum

  /**************************************
  * Section             /----\
  *                    /|    |\
  *        /+---------/ |    | \
  *       / |        |  |    |  \
  * -----/  |        |  |    |   \----
  *      PRE  MAIN   PRE MAIN POST PAUSE
  ***************************************/
  enum class Section : uchar { PRE, MAIN, POST, PAUSE };
  enum class FinishLevel : uchar { ZERO, NON_ZERO};

  ///////////////// Static
  const int JOINT_ARRAY_LENGTH = 22;
  static ushort startAngles1024[JOINT_ARRAY_LENGTH];
  static ushort targetAngles1024[JOINT_ARRAY_LENGTH];
  static short movingAngles1024[JOINT_ARRAY_LENGTH];
  static short mainAngles1024[JOINT_ARRAY_LENGTH];
  static short accelAngles1024[JOINT_ARRAY_LENGTH];
  static short mainSpeeds1024[JOINT_ARRAY_LENGTH];
  static short lastOutSpeeds1024[JOINT_ARRAY_LENGTH];
  static short goalSpeeds1024[JOINT_ARRAY_LENGTH];
  static FinishLevel finishTypes[JOINT_ARRAY_LENGTH];
  static ushort unitTimeCount;
  static ushort unitTimeNum;
  static ushort pauseTime;
  static ushort unitTimeTotalNum;
  static ushort accelStep;
  static Section section;
  static uchar playRepeatCount;
  static ushort nextPlayPage;

  auto hw = AgentState::get<HardwareState>();

  //
  // Initialise
  //

  if (d_isFirstStepOfAction)
  {
    // Special treatment for the first step of a new action
    d_isRunning = true;
    d_isFirstStepOfAction = false;
    d_playingFinished = false;
    d_isStopRequested = false;
    unitTimeCount = 0;
    unitTimeNum = 0;
    pauseTime = 0;
    section = Section::PAUSE;
    d_pageStepCount = 0;
    playRepeatCount = d_playingPage->getRepeatCount();
    nextPlayPage = 0;

    for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    {
        // Only update selected joints
      if (!(*selectedJoints)[jointId])
        continue;

      targetAngles1024[jointId] = hw->getMX28State(jointId)->presentPositionValue;
      lastOutSpeeds1024[jointId] = 0;
      movingAngles1024[jointId] = 0;
      goalSpeeds1024[jointId] = 0;
    }
  }

  if (unitTimeCount < unitTimeNum)
  {
    //
    // Continue current section
    //

    unitTimeCount++;
    if (section != Section::PAUSE)
    {
      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      {
        // Only update selected joints
        if (!(*selectedJoints)[jointId])
          continue;

        if (movingAngles1024[jointId] == 0)
        {
          d_values[jointId] = startAngles1024[jointId];
        }
        else switch (section)
        {
          case Section::PRE:
          {
            short iSpeedN = (short)(((long)(mainSpeeds1024[jointId] - lastOutSpeeds1024[jointId]) * unitTimeCount) / unitTimeNum);
            goalSpeeds1024[jointId] = lastOutSpeeds1024[jointId] + iSpeedN;
            accelAngles1024[jointId] =  (short)((((long)(lastOutSpeeds1024[jointId] + (iSpeedN >> 1)) * unitTimeCount * 144) / 15) >> 9);

            d_values[jointId] = startAngles1024[jointId] + accelAngles1024[jointId];
          }
          case Section::MAIN:
          {
            d_values[jointId] = startAngles1024[jointId] + (short)(((long)(mainAngles1024[jointId])*unitTimeCount) / unitTimeNum);
            goalSpeeds1024[jointId] = mainSpeeds1024[jointId];
          }
          case Section::POST:
          {
            if (unitTimeCount == (unitTimeNum-1))
            {
              d_values[jointId] = targetAngles1024[jointId];
            }
            else
            {
              if (finishTypes[jointId] == FinishLevel::ZERO)
              {
                short iSpeedN = (short)(((long)(0 - lastOutSpeeds1024[jointId]) * unitTimeCount) / unitTimeNum);
                goalSpeeds1024[jointId] = lastOutSpeeds1024[jointId] + iSpeedN;
                d_values[jointId] = startAngles1024[jointId] + (short)((((long)(lastOutSpeeds1024[jointId] + (iSpeedN>>1)) * unitTimeCount * 144) / 15) >> 9);
              }
              else // FinishLevel::NON_ZERO
              {
                // MAIN Section
                d_values[jointId] = startAngles1024[jointId] + (short)(((long)(mainAngles1024[jointId]) * unitTimeCount) / unitTimeNum);
                goalSpeeds1024[jointId] = mainSpeeds1024[jointId];
              }
            }
          }
          default:
          {
            throw new std::runtime_error("Unexpected section");
          }
        }

        d_pGains[jointId] = d_playingPage->getPGain(jointId);
      }
    }
  }
  else
  {
    // Completed previous section

    unitTimeCount = 0;

    for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    {
      if ((*selectedJoints)[jointId])
      {
        startAngles1024[jointId] = d_values[jointId];
        lastOutSpeeds1024[jointId] = goalSpeeds1024[jointId];
      }
    }

    // Section (PRE -> MAIN -> POST -> (PAUSE or PRE) ...)
    switch (section)
    {
      case Section::PRE:
      {
        section = Section::MAIN;
        unitTimeNum =  unitTimeTotalNum - (accelStep << 1);

        for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
        {
          if (!(*selectedJoints)[jointId])
            continue;

          switch (finishTypes[jointId])
          {
            case FinishLevel::NON_ZERO:
            {
              mainAngles1024[jointId] = (unitTimeTotalNum - accelStep) == 0
                ? mainAngles1024[jointId] = 0
                : mainAngles1024[jointId] = (short)((((long)(movingAngles1024[jointId] - accelAngles1024[jointId])) * unitTimeNum) / (unitTimeTotalNum - accelStep));
            }
            case FinishLevel::ZERO:
            {
              mainAngles1024[jointId] = movingAngles1024[jointId] - accelAngles1024[jointId] - (short)((((long)mainSpeeds1024[jointId] * accelStep * 12) / 5) >> 8);
            }
          }
        }
        break;
      }
      case Section::MAIN:
      {
        section = Section::POST;
        unitTimeNum = accelStep;

        for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
        {
          if ((*selectedJoints)[jointId])
            mainAngles1024[jointId] = movingAngles1024[jointId] - mainAngles1024[jointId] - accelAngles1024[jointId];
        }
        break;
      }
      case Section::POST:
      {
        if (pauseTime)
        {
          section = Section::PAUSE;
          unitTimeNum = pauseTime;
        }
        else
        {
          section = Section::PRE;
        }
        break;
      }
      case Section::PAUSE:
      {
        section = Section::PRE;
        memset(lastOutSpeeds1024, 0, sizeof(lastOutSpeeds1024));
        break;
      }
    }

    // PRE Section
    if (section == Section::PRE)
    {
      if (d_playingFinished)
      {
        d_isRunning = false;
        setCompletedFlag();
        return;
      }

      d_pageStepCount++;

      if (d_pageStepCount > d_playingPage->getStepCount())
      {
        d_playingPage = m_NextPlayPage;
        if (d_playingPageIndex != nextPlayPage)
          playRepeatCount = d_playingPage->getRepeatCount();
        d_pageStepCount = 1;
        d_playingPageIndex = nextPlayPage;
      }

      if (d_pageStepCount == d_playingPage->getStepCount())
      {
        if (d_isStopRequested)
        {
          nextPlayPage = d_playingPage->getExit();
        }
        else
        {
          playRepeatCount--;
          nextPlayPage = playRepeatCount > 0
            ? d_playingPageIndex
            : d_playingPage->getNext();
        }

        if (nextPlayPage == 0)
        {
          d_playingFinished = true;
        }
        else
        {
          if (d_playingPageIndex != nextPlayPage)
            m_NextPlayPage = d_file->getPageByIndex(nextPlayPage);
          else
            m_NextPlayPage = d_playingPage;

          if (m_NextPlayPage->getRepeatCount() == 0 || m_NextPlayPage->getStepCount() == 0)
            d_playingFinished = true;
        }
      }

      //////// Step
      pauseTime = (((ushort)d_playingPage->getStepPause(d_pageStepCount-1)) << 5) / d_playingPage->getSpeed();
      ushort wMaxSpeed256 = ((ushort)d_playingPage->getStepTime(d_pageStepCount-1) * (ushort)d_playingPage->getSpeed()) >> 5;
      if (wMaxSpeed256 == 0)
        wMaxSpeed256 = 1;
      ushort wMaxAngle1024 = 0;

      ////////// Joint
      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      {
        if (!(*selectedJoints)[jointId])
          continue;

        accelAngles1024[jointId] = 0;

        // Find current target angle
        ushort wCurrentTargetAngle = d_playingPage->getStepPosition(d_pageStepCount-1, jointId) & MotionScriptPage::INVALID_BIT_MASK
          ? targetAngles1024[jointId]
          : d_playingPage->getStepPosition(d_pageStepCount-1, jointId);

        // Update start, prev_target, curr_target
        startAngles1024[jointId] = targetAngles1024[jointId];
        ushort wPrevTargetAngle = targetAngles1024[jointId];
        targetAngles1024[jointId] = wCurrentTargetAngle;

        // Find Moving offset
        movingAngles1024[jointId] = (int)(targetAngles1024[jointId] - startAngles1024[jointId]);

        // Find Next target angle
        ushort wNextTargetAngle;
        if (d_pageStepCount == d_playingPage->getStepCount())
        {
          wNextTargetAngle = d_playingFinished
            ? wCurrentTargetAngle
            : m_NextPlayPage->getStepPosition(0, jointId) & MotionScriptPage::INVALID_BIT_MASK
              ? wCurrentTargetAngle
              : m_NextPlayPage->getStepPosition(0, jointId);
        }
        else
        {
          wNextTargetAngle = d_playingPage->getStepPosition(d_pageStepCount, jointId) & MotionScriptPage::INVALID_BIT_MASK
            ? wCurrentTargetAngle
            : d_playingPage->getStepPosition(d_pageStepCount, jointId);
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
        finishTypes[jointId] = bDirectionChanged || pauseTime || d_playingFinished
          ? FinishLevel::ZERO
          : FinishLevel::NON_ZERO;

        if (d_playingPage->getSchedule() == MotionScriptPageSchedule::SPEED_BASE)
        {
          // MaxAngle1024 update
          ushort wTmp = movingAngles1024[jointId] < 0
            ? -movingAngles1024[jointId]
            :  movingAngles1024[jointId];

          if (wTmp > wMaxAngle1024)
            wMaxAngle1024 = wTmp;
        }
      }

      //wUnitTimeNum = ((wMaxAngle1024*300/1024) /(wMaxSpeed256 * 720/256)) /7.8msec;
      //             = ((128*wMaxAngle1024*300/1024) /(wMaxSpeed256 * 720/256)) ;    (/7.8msec == *128)
      //             = (wMaxAngle1024*40) /(wMaxSpeed256 *3);
      unitTimeTotalNum = d_playingPage->getSchedule() == MotionScriptPageSchedule::TIME_BASE
        ? wMaxSpeed256 //TIME BASE 051025
        : (wMaxAngle1024 * 40) / (wMaxSpeed256 * 3);

      accelStep = d_playingPage->getAcceleration();
      if (unitTimeTotalNum <= (accelStep << 1))
      {
        if (unitTimeTotalNum == 0)
        {
          accelStep = 0;
        }
        else
        {
          accelStep = (unitTimeTotalNum - 1) >> 1;
          if (accelStep == 0)
            unitTimeTotalNum = 0;
        }
      }

      ulong ulTotalTime256T = ((ulong)unitTimeTotalNum) << 1;// /128 * 256
      ulong ulPreSectionTime256T = ((ulong)accelStep) << 1;// /128 * 256
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
          long lStartSpeed1024_PreTime_256T = (long)lastOutSpeeds1024[jointId] * ulPreSectionTime256T; //  *300/1024 * 1024/720 * 256 * 2
          long lMovingAngle_Speed1024Scale_256T_2T = (((long)movingAngles1024[jointId]) * 2560L) / 12;

          mainSpeeds1024[jointId] = finishTypes[jointId] == FinishLevel::ZERO
            ? (short)((lMovingAngle_Speed1024Scale_256T_2T - lStartSpeed1024_PreTime_256T) / lDivider2)
            : (short)((lMovingAngle_Speed1024Scale_256T_2T - lStartSpeed1024_PreTime_256T) / lDivider1);

          mainSpeeds1024[jointId] = Math::clamp(mainSpeeds1024[jointId], (short)-1023, (short)1023);
        }
      }

      unitTimeNum = accelStep;
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
