#include "actionmodule.hh"

#include "../../AgentState/agentstate.hh"
#include "../../BodyControl/bodycontrol.hh"
#include "../../MotionTaskScheduler/motiontaskscheduler.hh"
#include "../../MX28Snapshot/mx28snapshot.hh"
#include "../../StateObject/HardwareState/hardwarestate.hh"
#include "../../ThreadId/threadid.hh"

#include <iostream>
#include <string.h>

using namespace bold;
using namespace std;

ActionModule::ActionModule(std::shared_ptr<MotionTaskScheduler> scheduler, string filename)
: MotionModule("action", scheduler),
  d_file(nullptr),
  d_isRunning(false)
{
  loadFile(filename);
  
  for (string name : getPageNames())
    d_controls.push_back(Control::createAction(name, [this,name]() { start(name); }));
}

ActionModule::~ActionModule()
{
  if (d_file != 0)
    fclose(d_file);
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
  
  ulong ulTotalTime256T;
  ulong ulPreSectionTime256T;
  ulong ulMainTime256T;
  long lStartSpeed1024_PreTime_256T;
  long lMovingAngle_Speed1024Scale_256T_2T;
  long lDivider1,lDivider2;
  ushort wMaxAngle1024;
  ushort wMaxSpeed256;
  ushort wTmp;
  ushort wPrevTargetAngle; // Start position
  ushort wCurrentTargetAngle; // Target position
  ushort wNextTargetAngle; // Next target position
  uchar bDirectionChanged;

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
  short iSpeedN;
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
    d_stopRequested = false;
    wUnitTimeCount = 0;
    wUnitTimeNum = 0;
    wPauseTime = 0;
    bSection = PAUSE_SECTION;
    m_PageStepCount = 0;
    bPlayRepeatCount = d_playingPage.header.repeat;
    wNextPlayPage = 0;

    for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    {
      if ((*selectedJoints)[jointId])
      {
        wpTargetAngle1024[jointId] = hw->getMX28State(jointId)->presentPositionValue;
        ipLastOutSpeed1024[jointId] = 0;
        ipMovingAngle1024[jointId] = 0;
        ipGoalSpeed1024[jointId] = 0;
      }
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
    if (bSection == PAUSE_SECTION)
    {
    }
    else
    {
      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      {
        if ((*selectedJoints)[jointId])
        {
          if (ipMovingAngle1024[jointId] == 0)
            d_values[jointId] = wpStartAngle1024[jointId];
          else
          {
            if (bSection == PRE_SECTION)
            {
              iSpeedN = (short)(((long)(ipMainSpeed1024[jointId] - ipLastOutSpeed1024[jointId]) * wUnitTimeCount) / wUnitTimeNum);
              ipGoalSpeed1024[jointId] = ipLastOutSpeed1024[jointId] + iSpeedN;
              ipAccelAngle1024[jointId] =  (short)((((long)(ipLastOutSpeed1024[jointId] + (iSpeedN >> 1)) * wUnitTimeCount * 144) / 15) >> 9);

              d_values[jointId] = wpStartAngle1024[jointId] + ipAccelAngle1024[jointId];
            }
            else if (bSection == MAIN_SECTION)
            {
              d_values[jointId] = wpStartAngle1024[jointId] + (short)(((long)(ipMainAngle1024[jointId])*wUnitTimeCount) / wUnitTimeNum);
              ipGoalSpeed1024[jointId] = ipMainSpeed1024[jointId];
            }
            else // POST_SECTION
            {
              if (wUnitTimeCount == (wUnitTimeNum-1))
              {
                d_values[jointId] = wpTargetAngle1024[jointId];
              }
              else
              {
                if (bpFinishType[jointId] == ZERO_FINISH)
                {
                  iSpeedN = (short)(((long)(0 - ipLastOutSpeed1024[jointId]) * wUnitTimeCount) / wUnitTimeNum);
                  ipGoalSpeed1024[jointId] = ipLastOutSpeed1024[jointId] + iSpeedN;
                  d_values[jointId] = wpStartAngle1024[jointId] +  (short)((((long)(ipLastOutSpeed1024[jointId] + (iSpeedN>>1)) * wUnitTimeCount * 144) / 15) >> 9);
                }
                else // NON_ZERO_FINISH
                {
                  // MAIN Section
                  d_values[jointId] = wpStartAngle1024[jointId] + (short)(((long)(ipMainAngle1024[jointId]) * wUnitTimeCount) / wUnitTimeNum);
                  ipGoalSpeed1024[jointId] = ipMainSpeed1024[jointId];
                }
              }
            }
          }

          d_pGains[jointId] = (256 >> (d_playingPage.header.slope[jointId]>>4)) << 2;
          
          cout << "jointId=" << (int)jointId << " slope=" << (int)d_playingPage.header.slope[jointId] << " p=" << (int)d_pGains[jointId] << endl;
        }
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
        wpStartAngle1024[jointId] = hw->getMX28State(jointId)->presentPositionValue;
        ipLastOutSpeed1024[jointId] = ipGoalSpeed1024[jointId];
      }
    }

    // Section (PRE -> MAIN -> POST -> (PAUSE or PRE) ...)
    if (bSection == PRE_SECTION)
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
            if ((wUnitTimeTotalNum - wAccelStep) == 0)
              ipMainAngle1024[jointId] = 0;
            else
              ipMainAngle1024[jointId] = (short)((((long)(ipMovingAngle1024[jointId] - ipAccelAngle1024[jointId])) * wUnitTimeNum) / (wUnitTimeTotalNum - wAccelStep));
          }
          else // ZERO_FINISH
            ipMainAngle1024[jointId] = ipMovingAngle1024[jointId] - ipAccelAngle1024[jointId] - (short)((((long)ipMainSpeed1024[jointId] * wAccelStep * 12) / 5) >> 8);
        }
      }
    }
    else if (bSection == MAIN_SECTION)
    {
      // POST Section
      bSection = POST_SECTION;
      wUnitTimeNum = wAccelStep;

      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      {
        if ((*selectedJoints)[jointId])
          ipMainAngle1024[jointId] = ipMovingAngle1024[jointId] - ipMainAngle1024[jointId] - ipAccelAngle1024[jointId];
      }
    }
    else if (bSection == POST_SECTION)
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
    }
    else if (bSection == PAUSE_SECTION)
    {
      // PRE Section
      bSection = PRE_SECTION;

      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      {
        if ((*selectedJoints)[jointId])
          ipLastOutSpeed1024[jointId] = 0;
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

      if (m_PageStepCount > d_playingPage.header.stepnum)
      {
        d_playingPage = m_NextPlayPage;
        if (d_playingPageIndex != wNextPlayPage)
          bPlayRepeatCount = d_playingPage.header.repeat;
        m_PageStepCount = 1;
        d_playingPageIndex = wNextPlayPage;
      }

      if (m_PageStepCount == d_playingPage.header.stepnum)
      {
        if (d_stopRequested)
        {
          wNextPlayPage = d_playingPage.header.exit;
        }
        else
        {
          bPlayRepeatCount--;
          if (bPlayRepeatCount > 0)
            wNextPlayPage = d_playingPageIndex;
          else
            wNextPlayPage = d_playingPage.header.next;
        }

        if (wNextPlayPage == 0)
        {
          d_playingFinished = true;
        }
        else
        {
          if (d_playingPageIndex != wNextPlayPage)
            loadPage(wNextPlayPage, &m_NextPlayPage);
          else
            m_NextPlayPage = d_playingPage;

          if (m_NextPlayPage.header.repeat == 0 || m_NextPlayPage.header.stepnum == 0)
            d_playingFinished = true;
        }
      }

      //////// Step
      wPauseTime = (((ushort)d_playingPage.step[m_PageStepCount-1].pause) << 5) / d_playingPage.header.speed;
      wMaxSpeed256 = ((ushort)d_playingPage.step[m_PageStepCount-1].time * (ushort)d_playingPage.header.speed) >> 5;
      if (wMaxSpeed256 == 0)
        wMaxSpeed256 = 1;
      wMaxAngle1024 = 0;

      ////////// Joint
      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      {
        if (!(*selectedJoints)[jointId])
          continue;

        ipAccelAngle1024[jointId] = 0;

        // Find current target angle
        if (d_playingPage.step[m_PageStepCount-1].position[jointId] & INVALID_BIT_MASK)
          wCurrentTargetAngle = wpTargetAngle1024[jointId];
        else
          wCurrentTargetAngle = d_playingPage.step[m_PageStepCount-1].position[jointId];

        // Update start, prev_target, curr_target
        wpStartAngle1024[jointId] = wpTargetAngle1024[jointId];
        wPrevTargetAngle = wpTargetAngle1024[jointId];
        wpTargetAngle1024[jointId] = wCurrentTargetAngle;

        // Find Moving offset
        ipMovingAngle1024[jointId] = (int)(wpTargetAngle1024[jointId] - wpStartAngle1024[jointId]);

        // Find Next target angle
        if (m_PageStepCount == d_playingPage.header.stepnum)
        {
          if (d_playingFinished)
            wNextTargetAngle = wCurrentTargetAngle;
          else
          {
            if (m_NextPlayPage.step[0].position[jointId] & INVALID_BIT_MASK)
              wNextTargetAngle = wCurrentTargetAngle;
            else
              wNextTargetAngle = m_NextPlayPage.step[0].position[jointId];
          }
        }
        else
        {
          if (d_playingPage.step[m_PageStepCount].position[jointId] & INVALID_BIT_MASK)
            wNextTargetAngle = wCurrentTargetAngle;
          else
            wNextTargetAngle = d_playingPage.step[m_PageStepCount].position[jointId];
        }

        // Find direction change
        if (((wPrevTargetAngle < wCurrentTargetAngle) && (wCurrentTargetAngle < wNextTargetAngle))
         || ((wPrevTargetAngle > wCurrentTargetAngle) && (wCurrentTargetAngle > wNextTargetAngle)))
        {
          bDirectionChanged = 0;
        }
        else
        {
          bDirectionChanged = 1;
        }

        // Find finish type
        if (bDirectionChanged || wPauseTime || d_playingFinished)
        {
          bpFinishType[jointId] = ZERO_FINISH;
        }
        else
        {
          bpFinishType[jointId] = NON_ZERO_FINISH;
        }

        if (d_playingPage.header.schedule == SPEED_BASE_SCHEDULE)
        {
          //MaxAngle1024 update
          if (ipMovingAngle1024[jointId] < 0)
            wTmp = -ipMovingAngle1024[jointId];
          else
            wTmp = ipMovingAngle1024[jointId];

          if (wTmp > wMaxAngle1024)
            wMaxAngle1024 = wTmp;
        }
      }

      //wUnitTimeNum = ((wMaxAngle1024*300/1024) /(wMaxSpeed256 * 720/256)) /7.8msec;
      //             = ((128*wMaxAngle1024*300/1024) /(wMaxSpeed256 * 720/256)) ;    (/7.8msec == *128)
      //             = (wMaxAngle1024*40) /(wMaxSpeed256 *3);
      if (d_playingPage.header.schedule == TIME_BASE_SCHEDULE)
        wUnitTimeTotalNum  = wMaxSpeed256; //TIME BASE 051025
      else
        wUnitTimeTotalNum  = (wMaxAngle1024 * 40) / (wMaxSpeed256 * 3);

      wAccelStep = d_playingPage.header.accel;
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

      ulTotalTime256T = ((ulong)wUnitTimeTotalNum) << 1;// /128 * 256
      ulPreSectionTime256T = ((ulong)wAccelStep) << 1;// /128 * 256
      ulMainTime256T = ulTotalTime256T - ulPreSectionTime256T;
      lDivider1 = ulPreSectionTime256T + (ulMainTime256T << 1);
      lDivider2 = (ulMainTime256T << 1);

      if (lDivider1 == 0)
        lDivider1 = 1;

      if (lDivider2 == 0)
        lDivider2 = 1;

      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
      {
        if ((*selectedJoints)[jointId])
        {
          lStartSpeed1024_PreTime_256T = (long)ipLastOutSpeed1024[jointId] * ulPreSectionTime256T; //  *300/1024 * 1024/720 * 256 * 2
          lMovingAngle_Speed1024Scale_256T_2T = (((long)ipMovingAngle1024[jointId]) * 2560L) / 12;

          if (bpFinishType[jointId] == ZERO_FINISH)
            ipMainSpeed1024[jointId] = (short)((lMovingAngle_Speed1024Scale_256T_2T - lStartSpeed1024_PreTime_256T) / lDivider2);
          else
            ipMainSpeed1024[jointId] = (short)((lMovingAngle_Speed1024Scale_256T_2T - lStartSpeed1024_PreTime_256T) / lDivider1);

          ipMainSpeed1024[jointId] = Math::clamp(ipMainSpeed1024[jointId], (short)-1023, (short)1023);
        }
      }

      wUnitTimeNum = wAccelStep; //PreSection
    }
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
  if (pageIndex < 1 || pageIndex >= MAXNUM_PAGE)
  {
    cerr << "[ActionModule::Start] Invalid page index: " << pageIndex << endl;
    return false;
  }

  PAGE page;
  if (!loadPage(pageIndex, &page))
    return false;

  return start(pageIndex, &page);
}

bool ActionModule::start(string pageName)
{
  PAGE page;

  for (int index = 1; index < MAXNUM_PAGE; index++)
  {
    if (!loadPage(index, &page))
      return false;

    if (strcmp(pageName.c_str(), (char*)page.header.name) == 0)
      return start(index, &page);
  }
  
  return false;
}

bool ActionModule::start(int index, PAGE *page)
{
  if (d_isRunning)
  {
    cerr << "[ActionModule::Start] Cannot play page index " << index << " -- already playing index " << d_playingPageIndex << endl;
    return false;
  }

  d_playingPage = *page;

  if (d_playingPage.header.repeat == 0 || d_playingPage.header.stepnum == 0)
  {
    cerr << "[ActionModule::Start] Page index " << index << " has no steps to perform" << endl;
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
  d_stopRequested = true;
}

void ActionModule::brake()
{
  d_isRunning = false;
}

set<string> ActionModule::getPageNames()
{
  set<string> names;

  PAGE page;

  for (int index = 1; index < MAXNUM_PAGE; index++)
  {
    if (loadPage(index, &page))
    {
      string name((char*)page.header.name);
      if (name.size())
        names.insert(name);
    }
  }

  return names;
}

bool ActionModule::loadFile(string filename)
{
  FILE *file = fopen(filename.c_str(), "r+b");
  if (file == 0)
  {
    cerr << "[ActionModule::LoadFile] Can not open motion file: " << filename << endl;
    return false;
  }

  fseek(file, 0, SEEK_END);
  if (ftell(file) != (long)(sizeof(PAGE) * MAXNUM_PAGE))
  {
    cerr << "[ActionModule::LoadFile] Invalid motion file size: " << filename << endl;
    fclose(file);
    return false;
  }

  // Close any previously loaded file handle
  if (d_file != 0)
    fclose(d_file);

  d_file = file;
  return true;
}

bool ActionModule::createFile(string filename)
{
  FILE *action = fopen(filename.c_str(), "ab");
  if (action == 0)
  {
    cerr << "[ActionModule::CreateFile] Can not create ActionModule file: " << filename << endl;
    return false;
  }

  PAGE page;
  resetPage(&page);
  for (int i = 0; i < MAXNUM_PAGE; i++)
    fwrite(&page, 1, sizeof(PAGE), action);

  // Close any previously loaded file handle
  if (d_file != 0)
    fclose(d_file);

  d_file = action;
  return true;
}

bool ActionModule::loadPage(int index, PAGE *page)
{
  long position = (long)(sizeof(PAGE)*index);

  if (fseek(d_file, position, SEEK_SET) != 0)
  {
    cerr << "[ActionModule::LoadPage] Error seeking file position: " << position << endl;
    return false;
  }

  if (fread(page, 1, sizeof(PAGE), d_file) != sizeof(PAGE))
  {
    cerr << "[ActionModule::LoadPage] Error reading page index: " << index << endl;
    return false;
  }

  if (!verifyChecksum(page))
    resetPage(page);

  return true;
}

bool ActionModule::savePage(int index, PAGE *page)
{
  long position = (long)(sizeof(PAGE)*index);

  if (!verifyChecksum(page))
      setChecksum(page);

  if (fseek(d_file, position, SEEK_SET) != 0)
  {
    cerr << "[ActionModule::SavePage] Error seeking file position: " << position << endl;
    return false;
  }

  if (fwrite(page, 1, sizeof(PAGE), d_file) != sizeof(PAGE))
  {
    cerr << "[ActionModule::LoadPage] Error writing page index: " << index << endl;
    return false;
  }

  return true;
}

void ActionModule::resetPage(PAGE *pPage)
{
  uchar *pt = (uchar*)pPage;

  // TODO memset?
  for (unsigned int i = 0; i < sizeof(PAGE); i++)
  {
    *pt = 0x00;
    pt++;
  }

  pPage->header.schedule = TIME_BASE_SCHEDULE; // default time base
  pPage->header.repeat = 1;
  pPage->header.speed = 32;
  pPage->header.accel = 32;

  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    pPage->header.slope[jointId] = 0x55;

  for (int i = 0; i < MAXNUM_STEP; i++)
  {
    for (int j = 0; j < 31; j++)
      pPage->step[i].position[j] = INVALID_BIT_MASK;

    pPage->step[i].pause = 0;
    pPage->step[i].time = 0;
  }

  setChecksum(pPage);
}

bool ActionModule::verifyChecksum(PAGE *pPage)
{
  uchar checksum = 0x00;
  uchar *pt = (uchar*)pPage;

  for (unsigned int i = 0; i < sizeof(PAGE); i++)
  {
    checksum += *pt;
    pt++;
  }

  if (checksum != 0xff)
  {
    cerr << "[ActionModule::verifyChecksum] Page checksum is invalid" << endl;
    return false;
  }

  return true;
}

void ActionModule::setChecksum(PAGE *pPage)
{
  uchar checksum = 0x00;
  uchar *pt = (uchar*)pPage;

  pPage->header.checksum = 0x00;

  for (unsigned int i = 0; i < sizeof(PAGE); i++)
  {
    checksum += *pt;
    pt++;
  }

  pPage->header.checksum = (uchar)(0xff - checksum);
}
