#include "actionmodule.hh"

#include "../../BodyControl/bodycontrol.hh"
#include "../../AgentState/agentstate.hh"
#include "../../StateObject/HardwareState/hardwarestate.hh"
#include "../../MX28Snapshot/mx28snapshot.hh"

#include <iostream>
#include <string.h>

using namespace bold;
using namespace std;

ActionModule::ActionModule()
: d_file(nullptr),
  d_isRunning(false)
{}

ActionModule::~ActionModule()
{
  if (d_file != 0)
    fclose(d_file);
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

  for (int i = MIN_JOINT_ID; i <= MAX_JOINT_ID; i++)
    pPage->header.slope[i] = 0x55;

  for (int i = 0; i < MAXNUM_STEP; i++)
  {
    for (int j = 0; j < 31; j++)
      pPage->step[i].position[j] = INVALID_BIT_MASK;

    pPage->step[i].pause = 0;
    pPage->step[i].time = 0;
  }

  setChecksum(pPage);
}

void ActionModule::initialize()
{
  d_isRunning = false;

  // TODO do we need to copy the current pose somehow at this time?
//   for (int id = 1; id < NUMBER_OF_JOINTS; id++)
//     d_jointData.setValue(id, MotionStatus::m_CurrentJoints.GetValue(id));
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
  int index;
  PAGE page;

  for (index = 1; index < MAXNUM_PAGE; index++)
  {
    if (!loadPage(index, &page))
      return false;

    if (strcmp(pageName.c_str(), (char*)page.header.name) == 0)
      break;
  }

  return start(index, &page);
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
  m_FirstDrivingStart = true;
  d_isRunning = true;
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

bool ActionModule::isRunning()
{
  return d_isRunning;
}

bool ActionModule::isRunning(int *page, int *step)
{
  if (page != 0)
    *page = d_playingPageIndex;

  if (step != 0)
    *step = m_PageStepCount - 1;

  return isRunning();
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

void ActionModule::step(JointSelection const& selectedJoints)
{
  unsigned long ulTotalTime256T;
  unsigned long ulPreSectionTime256T;
  unsigned long ulMainTime256T;
  long lStartSpeed1024_PreTime_256T;
  long lMovingAngle_Speed1024Scale_256T_2T;
  long lDivider1,lDivider2;
  unsigned short wMaxAngle1024;
  unsigned short wMaxSpeed256;
  unsigned short wTmp;
  unsigned short wPrevTargetAngle; // Start position
  unsigned short wCurrentTargetAngle; // Target position
  unsigned short wNextTargetAngle; // Next target position
  uchar bDirectionChanged;

  ///////////////// Static
  static unsigned short wpStartAngle1024[JOINT_ARRAY_LENGTH];
  static unsigned short wpTargetAngle1024[JOINT_ARRAY_LENGTH];
  static short int ipMovingAngle1024[JOINT_ARRAY_LENGTH];
  static short int ipMainAngle1024[JOINT_ARRAY_LENGTH];
  static short int ipAccelAngle1024[JOINT_ARRAY_LENGTH];
  static short int ipMainSpeed1024[JOINT_ARRAY_LENGTH];
  static short int ipLastOutSpeed1024[JOINT_ARRAY_LENGTH];
  static short int ipGoalSpeed1024[JOINT_ARRAY_LENGTH];
  static uchar bpFinishType[JOINT_ARRAY_LENGTH];
  short int iSpeedN;
  static unsigned short wUnitTimeCount;
  static unsigned short wUnitTimeNum;
  static unsigned short wPauseTime;
  static unsigned short wUnitTimeTotalNum;
  static unsigned short wAccelStep;
  static uchar bSection;
  static uchar bPlayRepeatCount;
  static unsigned short wNextPlayPage;

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

  if (!d_isRunning)
    return;

  if (m_FirstDrivingStart)
  {
    m_FirstDrivingStart = false; //First Process end
    m_PlayingFinished = false;
    d_stopRequested = false;
    wUnitTimeCount = 0;
    wUnitTimeNum = 0;
    wPauseTime = 0;
    bSection = PAUSE_SECTION;
    m_PageStepCount = 0;
    bPlayRepeatCount = d_playingPage.header.repeat;
    wNextPlayPage = 0;

    for (uchar bID = MIN_JOINT_ID; bID <= MAX_JOINT_ID; bID++)
    {
      if (selectedJoints[bID])
      {
        wpTargetAngle1024[bID] = hw->getMX28State(bID)->presentPositionValue; // MotionStatus::m_CurrentJoints.GetValue(bID);
        ipLastOutSpeed1024[bID] = 0;
        ipMovingAngle1024[bID] = 0;
        ipGoalSpeed1024[bID] = 0;
      }
    }
  }

  if (wUnitTimeCount < wUnitTimeNum)
  {
    wUnitTimeCount++;
    if (bSection == PAUSE_SECTION)
    {
    }
    else
    {
      for (uchar bID = MIN_JOINT_ID; bID <= MAX_JOINT_ID; bID++)
      {
        if (selectedJoints[bID])
        {
          if (ipMovingAngle1024[bID] == 0)
            d_values[bID] = wpStartAngle1024[bID];
          else
          {
            if (bSection == PRE_SECTION)
            {
              iSpeedN = (short)(((long)(ipMainSpeed1024[bID] - ipLastOutSpeed1024[bID]) * wUnitTimeCount) / wUnitTimeNum);
              ipGoalSpeed1024[bID] = ipLastOutSpeed1024[bID] + iSpeedN;
              ipAccelAngle1024[bID] =  (short)((((long)(ipLastOutSpeed1024[bID] + (iSpeedN >> 1)) * wUnitTimeCount * 144) / 15) >> 9);

              d_values[bID] = wpStartAngle1024[bID] + ipAccelAngle1024[bID];
            }
            else if (bSection == MAIN_SECTION)
            {
              d_values[bID] = wpStartAngle1024[bID] + (short int)(((long)(ipMainAngle1024[bID])*wUnitTimeCount) / wUnitTimeNum);
              ipGoalSpeed1024[bID] = ipMainSpeed1024[bID];
            }
            else // POST_SECTION
            {
              if (wUnitTimeCount == (wUnitTimeNum-1))
              {
                d_values[bID] = wpTargetAngle1024[bID];
              }
              else
              {
                if (bpFinishType[bID] == ZERO_FINISH)
                {
                  iSpeedN = (short int)(((long)(0 - ipLastOutSpeed1024[bID]) * wUnitTimeCount) / wUnitTimeNum);
                  ipGoalSpeed1024[bID] = ipLastOutSpeed1024[bID] + iSpeedN;
                  d_values[bID] = wpStartAngle1024[bID] +  (short)((((long)(ipLastOutSpeed1024[bID] + (iSpeedN>>1)) * wUnitTimeCount * 144) / 15) >> 9);
                }
                else // NON_ZERO_FINISH
                {
                  // MAIN Section
                  d_values[bID] = wpStartAngle1024[bID] + (short int)(((long)(ipMainAngle1024[bID]) * wUnitTimeCount) / wUnitTimeNum);
                  ipGoalSpeed1024[bID] = ipMainSpeed1024[bID];
                }
              }
            }
          }

//           d_jointData.setSlope(bID, 1 << (d_playingPage.header.slope[bID]>>4), 1 << (d_playingPage.header.slope[bID]&0x0f));
          d_pGains[bID] = (256 >> (d_playingPage.header.slope[bID]>>4)) << 2;
        }
      }
    }
  }
  else if (wUnitTimeCount >= wUnitTimeNum) // Section
  {
    wUnitTimeCount = 0;

    for (uchar bID = MIN_JOINT_ID; bID <= MAX_JOINT_ID; bID++)
    {
      if (selectedJoints[bID])
      {
        wpStartAngle1024[bID] = hw->getMX28State(bID)->presentPositionValue;
        ipLastOutSpeed1024[bID] = ipGoalSpeed1024[bID];
      }
    }

    // Section ������Ʈ (PRE -> MAIN -> POST -> (PAUSE or PRE) ...)
    if (bSection == PRE_SECTION)
    {
      // MAIN Section
      bSection = MAIN_SECTION;
      wUnitTimeNum =  wUnitTimeTotalNum - (wAccelStep << 1);

      for (uchar bID = MIN_JOINT_ID; bID <= MAX_JOINT_ID; bID++)
      {
        if (selectedJoints[bID])
        {
          if (bpFinishType[bID] == NON_ZERO_FINISH)
          {
            if ((wUnitTimeTotalNum - wAccelStep) == 0)
              ipMainAngle1024[bID] = 0;
            else
              ipMainAngle1024[bID] = (short)((((long)(ipMovingAngle1024[bID] - ipAccelAngle1024[bID])) * wUnitTimeNum) / (wUnitTimeTotalNum - wAccelStep));
          }
          else // ZERO_FINISH
            ipMainAngle1024[bID] = ipMovingAngle1024[bID] - ipAccelAngle1024[bID] - (short int)((((long)ipMainSpeed1024[bID] * wAccelStep * 12) / 5) >> 8);
        }
      }
    }
    else if (bSection == MAIN_SECTION)
    {
      // POST Section
      bSection = POST_SECTION;
      wUnitTimeNum = wAccelStep;

      for (uchar bID = MIN_JOINT_ID; bID <= MAX_JOINT_ID; bID++)
      {
        if (selectedJoints[bID])
          ipMainAngle1024[bID] = ipMovingAngle1024[bID] - ipMainAngle1024[bID] - ipAccelAngle1024[bID];
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

      for (uchar bID = MIN_JOINT_ID; bID <= MAX_JOINT_ID; bID++)
      {
        if (selectedJoints[bID])
          ipLastOutSpeed1024[bID] = 0;
      }
    }

    // PRE Section
    if (bSection == PRE_SECTION)
    {
      if (m_PlayingFinished)
      {
        d_isRunning = false;
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
          m_PlayingFinished = true;
        else
        {
          if (d_playingPageIndex != wNextPlayPage)
            loadPage(wNextPlayPage, &m_NextPlayPage);
          else
            m_NextPlayPage = d_playingPage;

          if (m_NextPlayPage.header.repeat == 0 || m_NextPlayPage.header.stepnum == 0)
            m_PlayingFinished = true;
        }
      }

      //////// Step
      wPauseTime = (((unsigned short)d_playingPage.step[m_PageStepCount-1].pause) << 5) / d_playingPage.header.speed;
      wMaxSpeed256 = ((unsigned short)d_playingPage.step[m_PageStepCount-1].time * (unsigned short)d_playingPage.header.speed) >> 5;
      if (wMaxSpeed256 == 0)
        wMaxSpeed256 = 1;
      wMaxAngle1024 = 0;

      ////////// Joint
      for (uchar bID = MIN_JOINT_ID; bID <= MAX_JOINT_ID; bID++)
      {
        if (!selectedJoints[bID])
          continue;

        ipAccelAngle1024[bID] = 0;

        // Find current target angle
        if (d_playingPage.step[m_PageStepCount-1].position[bID] & INVALID_BIT_MASK)
          wCurrentTargetAngle = wpTargetAngle1024[bID];
        else
          wCurrentTargetAngle = d_playingPage.step[m_PageStepCount-1].position[bID];

        // Update start, prev_target, curr_target
        wpStartAngle1024[bID] = wpTargetAngle1024[bID];
        wPrevTargetAngle = wpTargetAngle1024[bID];
        wpTargetAngle1024[bID] = wCurrentTargetAngle;

        // Find Moving offset
        ipMovingAngle1024[bID] = (int)(wpTargetAngle1024[bID] - wpStartAngle1024[bID]);

        // Find Next target angle
        if (m_PageStepCount == d_playingPage.header.stepnum)
        {
          if (m_PlayingFinished)
            wNextTargetAngle = wCurrentTargetAngle;
          else
          {
            if (m_NextPlayPage.step[0].position[bID] & INVALID_BIT_MASK)
              wNextTargetAngle = wCurrentTargetAngle;
            else
              wNextTargetAngle = m_NextPlayPage.step[0].position[bID];
          }
        }
        else
        {
          if (d_playingPage.step[m_PageStepCount].position[bID] & INVALID_BIT_MASK)
            wNextTargetAngle = wCurrentTargetAngle;
          else
            wNextTargetAngle = d_playingPage.step[m_PageStepCount].position[bID];
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
        if (bDirectionChanged || wPauseTime || m_PlayingFinished)
        {
          bpFinishType[bID] = ZERO_FINISH;
        }
        else
        {
          bpFinishType[bID] = NON_ZERO_FINISH;
        }

        if (d_playingPage.header.schedule == SPEED_BASE_SCHEDULE)
        {
          //MaxAngle1024 update
          if (ipMovingAngle1024[bID] < 0)
            wTmp = -ipMovingAngle1024[bID];
          else
            wTmp = ipMovingAngle1024[bID];

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

      ulTotalTime256T = ((unsigned long)wUnitTimeTotalNum) << 1;// /128 * 256
      ulPreSectionTime256T = ((unsigned long)wAccelStep) << 1;// /128 * 256
      ulMainTime256T = ulTotalTime256T - ulPreSectionTime256T;
      lDivider1 = ulPreSectionTime256T + (ulMainTime256T << 1);
      lDivider2 = (ulMainTime256T << 1);

      if (lDivider1 == 0)
        lDivider1 = 1;

      if (lDivider2 == 0)
        lDivider2 = 1;

      for (uchar bID = MIN_JOINT_ID; bID <= MAX_JOINT_ID; bID++)
      {
        if (selectedJoints[bID])
        {
          lStartSpeed1024_PreTime_256T = (long)ipLastOutSpeed1024[bID] * ulPreSectionTime256T; //  *300/1024 * 1024/720 * 256 * 2
          lMovingAngle_Speed1024Scale_256T_2T = (((long)ipMovingAngle1024[bID]) * 2560L) / 12;

          if (bpFinishType[bID] == ZERO_FINISH)
            ipMainSpeed1024[bID] = (short int)((lMovingAngle_Speed1024Scale_256T_2T - lStartSpeed1024_PreTime_256T) / lDivider2);
          else
            ipMainSpeed1024[bID] = (short int)((lMovingAngle_Speed1024Scale_256T_2T - lStartSpeed1024_PreTime_256T) / lDivider1);

          if (ipMainSpeed1024[bID] > 1023)
            ipMainSpeed1024[bID] = 1023;

          if (ipMainSpeed1024[bID] < -1023)
            ipMainSpeed1024[bID] = -1023;
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
    joint->setValue(d_values[(int)joint->getId()]);
    joint->setPGain(d_pGains[(int)joint->getId()]);
  });
}

void ActionModule::applyHead(shared_ptr<HeadSection> head) { applySection(dynamic_pointer_cast<BodySection>(head)); }
void ActionModule::applyArms(shared_ptr<ArmSection> arms) { applySection(dynamic_pointer_cast<BodySection>(arms)); }
void ActionModule::applyLegs(shared_ptr<LegSection> legs) { applySection(dynamic_pointer_cast<BodySection>(legs)); }