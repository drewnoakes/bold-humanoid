#include "robotismotionfile.hh"

#include "../JointId/jointid.hh"
#include "../MotionScript/motionscript.hh"
#include "../util/log.hh"

#include <cstdio>
#include <vector>
#include <stdexcept>

#include <rapidjson/prettywriter.h>
#include <rapidjson/filewritestream.h>

using namespace bold;
using namespace std;
using namespace rapidjson;

RobotisMotionFile::RobotisMotionFile(string const& filePath)
{
  FILE *file = fopen(filePath.c_str(), "r+b");

  if (file == 0)
  {
    log::error("RobotisMotionFile::RobotisMotionFile") << "Can not open motion file: " << filePath;
    throw runtime_error("Can not open motion file");
  }

  fseek(file, 0, SEEK_END);

  long actualSize = ftell(file);
  long expectedSize = sizeof(RobotisMotionFile::Page) * ((int)MAX_PAGE_ID + 1);
  if (actualSize != expectedSize)
  {
    log::error("RobotisMotionFile::RobotisMotionFile") << "Invalid motion file size for " << filePath << " expecting " << expectedSize << " but got " << actualSize;
    fclose(file);
    throw runtime_error("Invalid motion file size");
  }

  // NOTE page zero is unused (all zeroed) and fails checksum validation, so skip it
  for (unsigned pageIndex = 1; pageIndex <= MAX_PAGE_ID; pageIndex++)
  {
    long position = (long)(sizeof(RobotisMotionFile::Page)*pageIndex);

    // TODO do we even have to seek if we read the file sequentially?
    if (fseek(file, position, SEEK_SET) != 0)
    {
      log::error("RobotisMotionFile::RobotisMotionFile") << "Error seeking file position: " << position;
      fclose(file);
      throw runtime_error("Error seeking file position");
    }

    if (fread(&d_pages[pageIndex], 1, sizeof(RobotisMotionFile::Page), file) != sizeof(RobotisMotionFile::Page))
    {
      log::error("RobotisMotionFile::RobotisMotionFile") << "Error reading page index: " << pageIndex;
      fclose(file);
      throw runtime_error("Error reading page");
    }

    if (!d_pages[pageIndex].isChecksumValid())
    {
      log::error("RobotisMotionFile::RobotisMotionFile") << "Checksum invalid for page index: " << pageIndex;
      fclose(file);
      throw runtime_error("Checksum invalid for page");
    }
  }

  fclose(file);
}

void RobotisMotionFile::toDotText(ostream& out) const
{
  out << "digraph MotionFile {" << endl;

  for (unsigned pageIndex = 0; pageIndex <= MAX_PAGE_ID; pageIndex++)
  {
    // Exit is an unused feature that allows graceful finish up after 'stop' requested, as opposed to brake.

    int nextIndex = d_pages[pageIndex].nextPageIndex;
    int repeatCount = d_pages[pageIndex].repeatCount;

    if (nextIndex)
      out << "    " << pageIndex << " -> " << nextIndex << ";" << endl;

    if (repeatCount > 1)
      out << "    " << pageIndex << " -> " << pageIndex << " [label=\"repeat " << repeatCount << "\"];" << endl;
  }

  for (unsigned pageIndex = 0; pageIndex <= MAX_PAGE_ID; pageIndex++)
  {
    auto name = d_pages[pageIndex].name;
    int stepCount = d_pages[pageIndex].stepCount;
    int speed = d_pages[pageIndex].speed;
    int acceleration = d_pages[pageIndex].accelerationTime;
    string schedule = d_pages[pageIndex].schedule == (uchar)Page::MotionScriptPageSchedule::SPEED_BASE ? "Speed" : "Time";

    if (stepCount != 0)
      out << "    " << pageIndex << " [label=\"" << name
          << "\\nspeed=" << speed << " sched=" << schedule
          << "\\nid=" << pageIndex << " steps=" << stepCount
          << "\\nacc=" << acceleration
          << "\"];" << endl;
  }

  out << "}" << endl;
}

vector<uchar> RobotisMotionFile::getSequenceRootPageIndices() const
{
  bool exists[(ushort)MAX_PAGE_ID + 1] = {0,};
  bool isTarget[(ushort)MAX_PAGE_ID + 1] = {0,};

  for (int pageIndex = 0; pageIndex <= MAX_PAGE_ID; pageIndex++)
  {
    auto page = d_pages[pageIndex];
    if (page.stepCount == 0)
      continue;
    exists[pageIndex] = true;
    if (page.nextPageIndex != 0)
      isTarget[page.nextPageIndex] = true;
  }

  vector<uchar> indices;
  for (int pageIndex = 0; pageIndex <= MAX_PAGE_ID; pageIndex++)
  {
    if (exists[pageIndex] && !isTarget[pageIndex])
      indices.push_back(pageIndex);
  }
  return indices;
}

shared_ptr<MotionScript> RobotisMotionFile::toMotionScript(uchar rootPageIndex)
{
  vector<shared_ptr<MotionScript::Stage>> stages;

  Page const* page = &d_pages[rootPageIndex];
  string name = page->name;
  shared_ptr<MotionScript::Stage> currentStage;

  bool startNewStage = true;
  while (true)
  {
    if (startNewStage)
    {
      startNewStage = false;
      currentStage = make_shared<MotionScript::Stage>();
      stages.push_back(currentStage);

      // NOTE we skip accelerationTime and schedule
      currentStage->repeatCount = page->repeatCount;
      currentStage->speed = page->speed;

      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
        currentStage->pGains[jointId - 1] = page->getPGain(jointId);
    }

    for (uchar i = 0; i < page->stepCount; i++)
    {
      auto step = MotionScript::KeyFrame();

      step.pauseCycles = page->steps[i].pause;
      step.moveCycles = page->steps[i].time;
      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
        step.values[jointId - 1] = page->steps[i].position[jointId];

      currentStage->keyFrames.push_back(step);
    }

    if (page->nextPageIndex == 0)
      break;

    // If next page and prior page share same parameters, don't start a new 'stage' object

    Page const& nextPage = d_pages[page->nextPageIndex];

    for (uchar j = (uchar)JointId::MIN; !startNewStage && j <= (uchar)JointId::MAX; j++)
      startNewStage |= page->slopes[j] != nextPage.slopes[j];

    startNewStage |=
        page->repeatCount != 1 ||
        nextPage.repeatCount != 1 ||
        nextPage.speed != page->speed ||
        nextPage.schedule != page->schedule ||
        nextPage.accelerationTime != page->accelerationTime;

    page = &nextPage;
  }

  return make_shared<MotionScript>(name, stages, true, true, true);
}

///////////////////////////////// PAGE FUNCTIONS

uchar RobotisMotionFile::Page::calculateChecksum() const
{
  uchar const* pt = (uchar const *)this;

  uchar checksum = 0x00;
  for (unsigned i = 0; i < sizeof(RobotisMotionFile::Page); i++)
  {
    checksum += *pt;
    pt++;
  }

  return checksum;
}

bool RobotisMotionFile::Page::isChecksumValid() const
{
  uchar calculated = calculateChecksum();

  // valid if the checksum calculation sums to 0xFF
  return calculated == 0xFF;
}

void RobotisMotionFile::Page::updateChecksum()
{
  // set to zero for the calculation
  checksum = 0x00;

  // calculate and update
  checksum = (uchar)(0xff - calculateChecksum());
}

void RobotisMotionFile::Page::reset()
{
  memset(this, 0, sizeof(RobotisMotionFile::Page));

  schedule = (uchar)MotionScriptPageSchedule::TIME_BASE; // default to time-base
  repeatCount = 1;
  speed = DEFAULT_SPEED;
  accelerationTime = DEFAULT_ACCELERATION;

  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    slopes[jointId] = DEFAULT_SLOPE;

  for (int i = 0; i < MAXNUM_STEPS; i++)
  {
    for (int j = 0; j < MAXNUM_POSITIONS; j++)
      steps[i].position[j] = INVALID_BIT_MASK;

    steps[i].pause = 0;
    steps[i].time = 0;
  }

  updateChecksum();
}
