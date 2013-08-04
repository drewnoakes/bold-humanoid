#include "motionscriptfile.hh"

#include "../MotionScriptPage/motionscriptpage.hh"

#include <iostream>
#include <cstdio>

using namespace bold;
using namespace std;

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
  // TODO
  return false;
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
