#include "motionscript.hh"

#include <rapidjson/prettywriter.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include <iostream>
#include <sstream>
#include <dirent.h>

using namespace bold;
using namespace std;
using namespace rapidjson;

// TODO consistent casing in JSON property names (p-gains / moveCycles)

shared_ptr<MotionScript> MotionScript::fromFile(std::string fileName)
{
  FILE* pFile = fopen(fileName.c_str(), "rb");
  if (!pFile)
  {
    cerr << "[MotionScript::fromFile] Unable to open file: " << fileName << endl;
    return nullptr;
  }

  char buffer[65536];
  FileReadStream is(pFile, buffer, sizeof(buffer));
  Document document;
  document.ParseStream<0, UTF8<>, FileReadStream>(is);

  if (document.HasParseError())
  {
    cerr << "[MotionScript::fromFile] JSON parse error for " << fileName << ": " << document.GetParseError() << endl;
    return nullptr;
  }

  auto name = document["name"].GetString();

  vector<shared_ptr<Stage>> stages;

  auto const& stagesArray = document["stages"];
  for (unsigned i = 0; i < stagesArray.Size(); i++)
  {
    auto const& stageMember = stagesArray[i];
    auto stage = make_shared<Stage>();

    stage->repeatCount = stageMember.TryGetIntValue("repeat", 1);

    stage->speed = stageMember.TryGetIntValue("speed", Stage::DEFAULT_SPEED);

    auto gainsMember = stageMember.FindMember("p-gains");
    if (gainsMember)
    {
      for (uchar g = 0; g < (uchar)JointId::MAX; g++)
        stage->pGains[g] = gainsMember->value[g].GetInt();
    }

    auto const& keyFrames = stageMember["keyFrames"];
    for (unsigned j = 0; j < keyFrames.Size(); j++)
    {
      auto const& keyFrameMember = keyFrames[j];
      auto keyFrame = KeyFrame();

      keyFrame.pauseCycles = keyFrameMember.TryGetIntValue("pauseCycles", 0);
      keyFrame.moveCycles = keyFrameMember["moveCycles"].GetInt();

      auto const& valuesMember = keyFrameMember["values"];
      for (uchar v = 0; v < (uchar)JointId::MAX; v++)
        keyFrame.values[v] = valuesMember[v].GetInt();

      stage->keyFrames.push_back(keyFrame);
    }

    stages.push_back(stage);
  }

  return make_shared<MotionScript>(name, stages);
}

vector<shared_ptr<MotionScript>> MotionScript::loadAllInPath(std::string path)
{
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir(path.c_str())) == nullptr)
  {
    cerr << "Unable to open motion scripts directory" << endl;
    throw std::runtime_error("Unable to open motion scripts directory");
  }

  vector<shared_ptr<MotionScript>> scripts;

  while ((ent = readdir(dir)) != nullptr)
  {
    // Skip anything that's not a file
    if (ent->d_type != DT_REG)
      continue;

    // TODO verify the file ends with .json

    stringstream filePath;
    filePath << path << "/" << ent->d_name;
    cout << "Found: " << filePath.str() << endl;
    scripts.push_back(fromFile(filePath.str()));
  }

  closedir(dir);

  return scripts;
}

bool MotionScript::writeJsonFile(std::string fileName) const
{
  FILE *file = fopen(fileName.c_str(), "wb");

  if (file == 0)
  {
    cerr << "[MotionScript::writeJsonFile] Can not open output file for writing: " << fileName << endl;
    return false;
  }

  cout << "[MotionScript::writeJsonFile] Writing: " << fileName << endl;

  char buffer[4096];
  FileWriteStream f(file, buffer, sizeof(buffer));
  PrettyWriter<FileWriteStream> writer(f);
  writer.SetIndent(' ', 2);

  writeJson(writer);

  f.Flush();

  fclose(file);

  return true;
}

void MotionScript::writeJson(PrettyWriter<FileWriteStream>& writer) const
{
  writer.StartObject();
  {
    writer.String("name").String(d_name.c_str());

    writer.String("stages");
    writer.StartArray();
    {
      for (std::shared_ptr<Stage> stage : d_stages)
      {
        writer.StartObject();
        {
          if (stage->repeatCount != 1)
            writer.String("repeat").Int(stage->repeatCount);

          if (stage->speed != Stage::DEFAULT_SPEED)
            writer.String("speed").Int(stage->speed);

          for (int jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
          {
            if (stage->pGains[jointId-1] != Stage::DEFAULT_P_GAIN)
            {
              writer.String("p-gains");
              writer.StartArray();
              for (int j = (uchar)JointId::MIN; j <= (uchar)JointId::MAX; j++)
                writer.Int(stage->pGains[j-1]);
              writer.EndArray();
              break;
            }
          }

          writer.String("keyFrames");
          writer.StartArray();
          {
            for (KeyFrame const& step : stage->keyFrames)
            {
              writer.StartObject();
              {
                if (step.pauseCycles != 0)
                  writer.String("pauseCycles").Int(step.pauseCycles);
                if (step.moveCycles != 0) // TODO a zero value here should be an error
                  writer.String("moveCycles").Int(step.moveCycles);
                writer.String("values").StartArray();
                for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
                  writer.Int(step.values[jointId-1]);
                writer.EndArray();
              }
              writer.EndObject();
            }
          }
          writer.EndArray();
        }
        writer.EndObject();
      }
    }
    writer.EndArray();
  }
  writer.EndObject();
}
