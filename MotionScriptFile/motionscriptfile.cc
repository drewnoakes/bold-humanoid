#include "motionscriptfile.hh"

#include "../MotionScriptPage/motionscriptpage.hh"
#include "../JointId/jointid.hh"

#include <iostream>
#include <cstdio>
#include <rapidjson/prettywriter.h>
#include <rapidjson/filewritestream.h>

using namespace bold;
using namespace std;
using namespace rapidjson;

shared_ptr<MotionScriptFile> MotionScriptFile::loadFromBinaryFile(string const& filePath)
{
  FILE *file = fopen(filePath.c_str(), "r+b");

  if (file == 0)
  {
    cerr << "[MotionScriptFile::loadFromBinaryFile] Can not open motion file: " << filePath << endl;
    return nullptr;
  }

  fseek(file, 0, SEEK_END);

  long actualSize = ftell(file);
  long expectedSize = sizeof(MotionScriptPage) * ((int)MAX_PAGE_ID + 1);
  if (actualSize != expectedSize)
  {
    cerr << "[MotionScriptFile::loadFromBinaryFile] Invalid motion file size for " << filePath << " expecting " << expectedSize << " but got " << actualSize << endl;
    fclose(file);
    return nullptr;
  }

  shared_ptr<MotionScriptPage> pages[(int)MAX_PAGE_ID + 1];

  for (unsigned pageIndex = 0; pageIndex <= MAX_PAGE_ID; pageIndex++)
  {
    long position = (long)(sizeof(MotionScriptPage)*pageIndex);

    // TODO do we even have to seek if we read the file sequentially?
    if (fseek(file, position, SEEK_SET) != 0)
    {
      cerr << "[MotionScriptFile::loadFromBinaryFile] Error seeking file position: " << position << endl;
      fclose(file);
      return nullptr;
    }

    pages[pageIndex] = make_shared<MotionScriptPage>();

    if (fread(pages[pageIndex].get(), 1, sizeof(MotionScriptPage), file) != sizeof(MotionScriptPage))
    {
      cerr << "[MotionScriptFile::loadFromBinaryFile] Error reading page index: " << pageIndex << endl;
      fclose(file);
      return nullptr;
    }

    if (!pages[pageIndex]->isChecksumValid())
    {
      cerr << "[MotionScriptFile::loadFromBinaryFile] Checksum invalid for page index: " << pageIndex << endl;
      fclose(file);
      return nullptr;
    }
  }

  fclose(file);

  return make_shared<MotionScriptFile>(pages);
}

shared_ptr<MotionScriptFile> MotionScriptFile::loadFromJsonFile(string const& filePath)
{
  // TODO
  return nullptr;
}

bool MotionScriptFile::saveToBinaryFile(string const& filePath) const
{
  FILE *file = fopen(filePath.c_str(), "ab");
  if (file == 0)
  {
    cerr << "[MotionScriptFile::saveToBinaryFile] Can not create file: " << filePath << endl;
    return false;
  }

  for (int pageIndex = 0; pageIndex <= MAX_PAGE_ID; pageIndex++)
  {
    d_pages[pageIndex]->updateChecksum();

    if (fwrite(d_pages[pageIndex].get(), 1, sizeof(MotionScriptPage), file) != sizeof(MotionScriptPage))
    {
      cerr << "[MotionScriptFile::saveToBinaryFile] Error writing page index: " << pageIndex << endl;
      return false;
    }
  }

  fclose(file);
  return true;
}

vector<shared_ptr<MotionScriptPage>> MotionScriptFile::getSequenceRoots() const
{
  bool exists[(ushort)MAX_PAGE_ID + 1] = {0,};
  bool isTarget[(ushort)MAX_PAGE_ID + 1] = {0,};

  for (int pageIndex = 0; pageIndex <= MAX_PAGE_ID; pageIndex++)
  {
    auto page = d_pages[pageIndex];
    if (page->getStepCount() == 0)
      continue;
    exists[pageIndex] = true;
    if (page->getNext() != 0)
      isTarget[page->getNext()] = true;
  }

  vector<shared_ptr<MotionScriptPage>> rootPages;
  for (int pageIndex = 0; pageIndex <= MAX_PAGE_ID; pageIndex++)
  {
    if (exists[pageIndex] && !isTarget[pageIndex])
      rootPages.push_back(d_pages[pageIndex]);
  }
  return rootPages;
}

bool MotionScriptFile::saveToJsonFile(string const& filePath) const
{
  // NOTE this method is a transitory solution as we move away from using binary files

  FILE *file = fopen(filePath.c_str(), "wb");

  if (file == 0)
  {
    cerr << "[MotionScriptFile::saveToJsonFile] Can not open output file for writing: " << filePath << endl;
    return false;
  }

  char buffer[1024];
  FileWriteStream f(file, buffer, 1024);
  PrettyWriter<FileWriteStream> writer(f);
  writer.SetIndent(' ', 2);

  writer.StartArray();
  for (shared_ptr<MotionScriptPage> rootPage : getSequenceRoots())
  {
    writer.StartObject();
    {
      writer.String("name").String(rootPage->getName().c_str());

      shared_ptr<MotionScriptPage> page = rootPage;

      writer.String("stages");
      writer.StartArray();
      {
        bool startNewStage = true;
        while (true)
        {
          if (startNewStage)
          {
            writer.StartObject();
            startNewStage = false;

            if (page->getRepeatCount() != 1)
              writer.String("repeat").Int(page->getRepeatCount());
            if (page->getSpeed() != MotionScriptPage::DEFAULT_SPEED)
              writer.String("speed").Int(page->getSpeed());
            if (page->getAcceleration() != MotionScriptPage::DEFAULT_ACCELERATION)
              writer.String("acceleration").Int(page->getAcceleration());
            if (page->getSchedule() != MotionScriptPage::DEFAULT_SCHEDULE)
              writer.String("schedule").String(page->getSchedule() == MotionScriptPageSchedule::SPEED_BASE ? "speed" : "time");

            for (int i = (uchar)JointId::MIN; i <= (uchar)JointId::MAX; i++)
            {
              if (page->getSlope(i) != MotionScriptPage::DEFAULT_SLOPE)
              {
                writer.String("p-gains");
                writer.StartArray();
                for (int j = (uchar)JointId::MIN; j <= (uchar)JointId::MAX; j++)
                  writer.Int(page->getPGain(j));
                writer.EndArray();
                break;
              }
            }

            writer.String("steps");
            writer.StartArray();
          }

          for (uchar i = 0; i < page->getStepCount(); i++)
          {
            writer.StartObject();
            {
              if (page->getStepPause(i) != 0)
                writer.String("pauseCycles").Int(page->getStepPause(i));
              if (page->getStepTime(i) != 0)
                writer.String("moveCycles").Int(page->getStepTime(i));
              writer.String("values").StartArray();
              for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
                writer.Int(page->getStepPosition(i, jointId));
              writer.EndArray();
            }
            writer.EndObject();
          }

          if (page->getNext() == 0)
          {
            writer.EndArray();
            writer.EndObject();
            break;
          }

          // If next page and prior page share same parameters, don't start a new 'stage' object

          auto nextPage = getPageByIndex(page->getNext());

          for (int j = (uchar)JointId::MIN; !startNewStage && j <= (uchar)JointId::MAX; j++)
            startNewStage |= page->getSlope(j) != nextPage->getSlope(j);

          startNewStage |=
              page->getRepeatCount() != 1 ||
              nextPage->getRepeatCount() != 1 ||
              nextPage->getSpeed() != page->getSpeed() ||
              nextPage->getSchedule() != page->getSchedule() ||
              nextPage->getAcceleration() != page->getAcceleration();

          page = nextPage;

          if (startNewStage)
          {
            writer.EndArray();
            writer.EndObject();
          }
        }
      }
      writer.EndArray();
    }
    writer.EndObject();
  }
  writer.EndArray();

  f.Flush();

  fclose(file);

  return true;
}

MotionScriptFile::MotionScriptFile()
{
  for (int pageIndex = 0; pageIndex <= MAX_PAGE_ID; pageIndex++)
  {
    d_pages[pageIndex] = make_shared<MotionScriptPage>();
    d_pages[pageIndex]->reset();
  }
}

MotionScriptFile::MotionScriptFile(shared_ptr<MotionScriptPage> pages[(ushort)MAX_PAGE_ID + 1])
{
  for (int i = 0; i <= MAX_PAGE_ID; i++)
    d_pages[i] = pages[i];
}

set<string> MotionScriptFile::getPageNames() const
{
  set<string> names;

  for (int index = 0; index <= MAX_PAGE_ID; index++)
  {
    auto page = d_pages[index];
    string name = page->getName();
    if (name.size())
      names.insert(name);
  }

  return names;
}

shared_ptr<MotionScriptPage> MotionScriptFile::getPageByIndex(uchar pageIndex) const
{
  return d_pages[pageIndex];
}

shared_ptr<MotionScriptPage> MotionScriptFile::getPageByName(string const& pageName) const
{
  for (int index = 0; index <= MAX_PAGE_ID; index++)
  {
    auto page = d_pages[index];
    if (page->getName() == pageName)
      return page;
  }
  return nullptr;
}

void MotionScriptFile::toDotText(ostream& out) const
{
  out << "digraph MotionFile {" << endl;

  for (unsigned pageIndex = 0; pageIndex <= MAX_PAGE_ID; pageIndex++)
  {
    int nextIndex = d_pages[pageIndex]->getNext();
    int repeatCount = d_pages[pageIndex]->getRepeatCount();
    int exitIndex = d_pages[pageIndex]->getExit();

    if (nextIndex)
      out << "    " << pageIndex << " -> " << nextIndex << ";" << endl;

    if (repeatCount > 1)
      out << "    " << pageIndex << " -> " << pageIndex << " [label=\"repeat " << repeatCount << "\"];" << endl;

    if (exitIndex)
      out << "    " << pageIndex << " -> " << exitIndex << " [label=\"exit\"];" << endl;
  }

  for (unsigned pageIndex = 0; pageIndex <= MAX_PAGE_ID; pageIndex++)
  {
    auto name = d_pages[pageIndex]->getName();
    int stepCount = d_pages[pageIndex]->getStepCount();
    int speed = d_pages[pageIndex]->getSpeed();
    int acceleration = d_pages[pageIndex]->getAcceleration();
    string schedule = d_pages[pageIndex]->getSchedule() == MotionScriptPageSchedule::SPEED_BASE ? "Speed" : "Time";

    if (stepCount != 0)
      out << "    " << pageIndex << " [label=\"" << name
          << "\\nspeed=" << speed << " sched=" << schedule
          << "\\nid=" << pageIndex << " steps=" << stepCount
          << "\\nacc=" << acceleration
          << "\"];" << endl;
  }

  out << "}" << endl;
}

int MotionScriptFile::indexOf(shared_ptr<MotionScriptPage> page) const
{
  for (unsigned pageIndex = 0; pageIndex <= MAX_PAGE_ID; pageIndex++)
  {
    if (page.get() == d_pages[pageIndex].get())
      return pageIndex;
  }
  return -1;
}
