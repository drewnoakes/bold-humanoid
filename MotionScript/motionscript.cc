#include "motionscript.hh"

#include "../util/assert.hh"
#include "../util/log.hh"

#include <sstream>
#include <dirent.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

using namespace bold;
using namespace std;
using namespace rapidjson;

// TODO consistent casing in JSON property names (p-gains / moveCycles)

shared_ptr<MotionScript> MotionScript::fromFile(string fileName)
{
  FILE* pFile = fopen(fileName.c_str(), "rb");
  if (!pFile)
  {
    log::error("MotionScript::fromFile") << "Unable to open file: " << fileName;
    return nullptr;
  }

  char buffer[65536];
  FileReadStream is(pFile, buffer, sizeof(buffer));
  Document document;
  document.ParseStream<0, UTF8<>, FileReadStream>(is);

  if (document.HasParseError())
  {
    log::error("MotionScript::fromFile") << "JSON parse error for " << fileName << ": " << document.GetParseError();
    return nullptr;
  }

  auto name = document["name"].GetString();

  bool controlsHead = document.TryGetBoolValue("controls-head", true);
  bool controlsArms = document.TryGetBoolValue("controls-arms", true);
  bool controlsLegs = document.TryGetBoolValue("controls-legs", true);

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

      if (keyFrame.moveCycles < 3)
      {
        // NOTE the MotionScriptRunner hits an arithmetic error (div by zero) if
        //      moveCycles is less than 3. Should really fix the bug there, but
        //      this is easier for now and provides quick feedback.
        log::error("MotionScript::fromFile") << "moveCycles value must be greater than 2 in for " << fileName;
        return nullptr;
      }

      auto const& valuesMember = keyFrameMember["values"];
      for (uchar v = 0; v < (uchar)JointId::MAX; v++)
        keyFrame.values[v] = valuesMember[v].GetInt();

      stage->keyFrames.push_back(keyFrame);
    }

    stages.push_back(stage);
  }

  return make_shared<MotionScript>(name, stages, controlsHead, controlsArms, controlsLegs);
}

vector<shared_ptr<MotionScript>> MotionScript::loadAllInPath(string path)
{
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir(path.c_str())) == nullptr)
  {
    log::error("MotionScript::loadAllInPath") << "Unable to open motion scripts directory";
    throw runtime_error("Unable to open motion scripts directory");
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
    scripts.push_back(fromFile(filePath.str()));
  }

  closedir(dir);

  return scripts;
}

bool MotionScript::writeJsonFile(string fileName) const
{
  FILE *file = fopen(fileName.c_str(), "wb");

  if (file == 0)
  {
    log::error("MotionScript::writeJsonFile") << "Can not open output file for writing: " << fileName;
    return false;
  }

  log::info("MotionScript::writeJsonFile") << "Writing: " << fileName;

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
      for (shared_ptr<Stage> stage : d_stages)
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

shared_ptr<MotionScript::Stage const> MotionScript::getFinalStage() const
{
  ASSERT(d_stages.size());
  return d_stages[d_stages.size() - 1];
}

shared_ptr<MotionScript::Stage const> MotionScript::getFirstStage() const
{
  ASSERT(d_stages.size());
  return d_stages[0];
}

MotionScript::KeyFrame const& MotionScript::getFinalKeyFrame() const
{
  auto finalStage = getFinalStage();
  ASSERT(finalStage->keyFrames.size());
  return finalStage->keyFrames[finalStage->keyFrames.size() - 1];
}
