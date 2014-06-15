#include "motionscript.hh"

#include "../util/assert.hh"
#include "../util/log.hh"

#include <sstream>
#include <dirent.h>

#include <rapidjson/filereadstream.h>
#include <rapidjson/filestream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>

using namespace bold;
using namespace std;
using namespace rapidjson;

const uchar MotionScript::Stage::DEFAULT_SPEED = 32;
const uchar MotionScript::Stage::DEFAULT_P_GAIN = 32;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

shared_ptr<MotionScript> MotionScript::fromFile(string fileName)
{
  FILE* file = fopen(fileName.c_str(), "rb");
  if (!file)
  {
    log::error("MotionScript::fromFile") << "Unable to open file \"" << fileName << "\": " << strerror(errno) << " (" << errno << ")";
    return nullptr;
  }

  char buffer[65536];
  FileReadStream is(file, buffer, sizeof(buffer));
  Document document;
  document.ParseStream<0, UTF8<>, FileReadStream>(is);

  if (fclose(file) != 0)
    log::warning("MotionScript::fromFile") << "Error closing file \"" << fileName << "\": " << strerror(errno) << " (" << errno << ")";

  if (document.HasParseError())
  {
    log::error("MotionScript::fromFile") << "JSON parse error for " << fileName << ": " << document.GetParseError();
    return nullptr;
  }

  auto name = document["name"].GetString();

  bool controlsHead = document.TryGetBoolValue("controlsHead", true);
  bool controlsArms = document.TryGetBoolValue("controlsArms", true);
  bool controlsLegs = document.TryGetBoolValue("controlsLegs", true);

  vector<shared_ptr<Stage>> stages;

  auto const& stagesArray = document["stages"];
  for (unsigned i = 0; i < stagesArray.Size(); i++)
  {
    auto const& stageMember = stagesArray[i];
    auto stage = make_shared<Stage>();

    stage->repeatCount = (uchar)stageMember.TryGetUintValue("repeat", 1);
    stage->speed       = (uchar)stageMember.TryGetUintValue("speed", Stage::DEFAULT_SPEED);

    auto gainsMember = stageMember.FindMember("pGains");
    if (gainsMember)
    {
      for (uchar g = 0; g < (uchar)JointId::MAX; g++)
      {
        // Try to find a paired gain entry
        if (JointPairs::isBase(g + (uchar)1))
        {
          string pairName = JointName::getJsonPairName(g + (uchar)1);
          auto pairMember = gainsMember->value.FindMember(pairName.c_str());
          if (pairMember)
          {
            // If a pair exists, the individual gains shouldn't
            if (gainsMember->value.FindMember(JointName::getJsonName(g + (uchar)1).c_str()) ||
                gainsMember->value.FindMember(JointName::getJsonName(g + (uchar)2).c_str()))
            {
              log::error("MotionScript::fromFile") << "JSON file " << fileName << " specifies pGain pair name '" << pairName << "' but also specifies L or R property in same stage";
              throw std::runtime_error("JSON specifies pGain pair name and also individual L/R joint in same stage");
            }

            uchar value = (uchar)pairMember->value.GetUint();

            stage->pGains[g] = value;
            stage->pGains[g+1] = value;

            g++;
            continue;
          }
        }

        stage->pGains[g] = (uchar)gainsMember->value.TryGetUintValue(JointName::getJsonName(g + 1).c_str(), Stage::DEFAULT_P_GAIN);
      }
    }

    auto const& keyFrames = stageMember["keyFrames"];
    for (unsigned j = 0; j < keyFrames.Size(); j++)
    {
      auto const& keyFrameMember = keyFrames[j];
      auto keyFrame = KeyFrame();

      keyFrame.pauseCycles = (uchar)keyFrameMember.TryGetUintValue("pauseCycles", 0u);
      keyFrame.moveCycles = (uchar)keyFrameMember["moveCycles"].GetUint();

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
      {
        // Try to find a paired value
        if (JointPairs::isBase(v + (uchar)1))
        {
          string pairName = JointName::getJsonPairName(v + (uchar)1);
          auto pairMember = valuesMember.FindMember(pairName.c_str());
          if (pairMember)
          {
            // If a pair exists, the individual values shouldn't
            if (valuesMember.FindMember(JointName::getJsonName(v + (uchar)1).c_str()) ||
                valuesMember.FindMember(JointName::getJsonName(v + (uchar)2).c_str()))
            {
              log::error("MotionScript::fromFile") << "JSON file " << fileName << " specifies value pair name '" << pairName << "' but also specifies L or R property in same stage";
              throw std::runtime_error("JSON specifies value pair name and also individual L/R joint in same stage");
            }

            ushort value = (ushort)pairMember->value.GetUint();

            keyFrame.values[v] = value;
            keyFrame.values[v+1] = MX28::getMirrorValue(value);

            v++;
            continue;
          }
        }

        string propName = JointName::getJsonName(v + (uchar)1);
        auto prop = valuesMember.FindMember(propName.c_str());
        if (!prop)
        {
          log::error("MotionScript::fromFile") << "Missing property " << propName << " in file " << fileName;
          return nullptr;
        }
        keyFrame.values[v] = (ushort)prop->value.GetUint();
      }

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
    auto script = fromFile(filePath.str());
    if (script != nullptr)
      scripts.push_back(script);
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

  f.Put('\n');

  f.Flush();

  if (fclose(file) != 0)
    log::warning("MotionScript::writeJsonFile") << "Error closing file \"" << fileName << "\": " << strerror(errno) << " (" << errno << ")";

  return true;
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

shared_ptr<MotionScript> MotionScript::getMirroredScript(std::string name) const
{
  // Copy the script
  auto mirror = make_shared<MotionScript>(*this);

  // Set the new name
  mirror->setName(name);

  for (auto& stage : mirror->d_stages)
  {
    // Transpose p-gains across body
    std::swap(stage->pGains.at((uchar)JointId::L_ELBOW - 1),          stage->pGains.at((uchar)JointId::R_ELBOW - 1));
    std::swap(stage->pGains.at((uchar)JointId::L_SHOULDER_PITCH - 1), stage->pGains.at((uchar)JointId::R_SHOULDER_PITCH - 1));
    std::swap(stage->pGains.at((uchar)JointId::L_SHOULDER_ROLL - 1),  stage->pGains.at((uchar)JointId::R_SHOULDER_ROLL - 1));
    std::swap(stage->pGains.at((uchar)JointId::L_ANKLE_PITCH - 1),    stage->pGains.at((uchar)JointId::R_ANKLE_PITCH - 1));
    std::swap(stage->pGains.at((uchar)JointId::L_ANKLE_ROLL - 1),     stage->pGains.at((uchar)JointId::L_ANKLE_ROLL - 1));
    std::swap(stage->pGains.at((uchar)JointId::L_KNEE - 1),           stage->pGains.at((uchar)JointId::R_KNEE - 1));
    std::swap(stage->pGains.at((uchar)JointId::L_HIP_YAW - 1),        stage->pGains.at((uchar)JointId::R_HIP_YAW - 1));
    std::swap(stage->pGains.at((uchar)JointId::L_HIP_PITCH - 1),      stage->pGains.at((uchar)JointId::R_HIP_PITCH - 1));
    std::swap(stage->pGains.at((uchar)JointId::L_HIP_ROLL - 1),       stage->pGains.at((uchar)JointId::R_HIP_ROLL - 1));

    for (auto& keyFrame : stage->keyFrames)
    {
      // Transpose values across body
      std::swap(keyFrame.values.at((uchar)JointId::L_ELBOW - 1),          keyFrame.values.at((uchar)JointId::R_ELBOW - 1));
      std::swap(keyFrame.values.at((uchar)JointId::L_SHOULDER_PITCH - 1), keyFrame.values.at((uchar)JointId::R_SHOULDER_PITCH - 1));
      std::swap(keyFrame.values.at((uchar)JointId::L_SHOULDER_ROLL - 1),  keyFrame.values.at((uchar)JointId::R_SHOULDER_ROLL - 1));
      std::swap(keyFrame.values.at((uchar)JointId::L_ANKLE_PITCH - 1),    keyFrame.values.at((uchar)JointId::R_ANKLE_PITCH - 1));
      std::swap(keyFrame.values.at((uchar)JointId::L_ANKLE_ROLL - 1),     keyFrame.values.at((uchar)JointId::L_ANKLE_ROLL - 1));
      std::swap(keyFrame.values.at((uchar)JointId::L_KNEE - 1),           keyFrame.values.at((uchar)JointId::R_KNEE - 1));
      std::swap(keyFrame.values.at((uchar)JointId::L_HIP_YAW - 1),        keyFrame.values.at((uchar)JointId::R_HIP_YAW - 1));
      std::swap(keyFrame.values.at((uchar)JointId::L_HIP_PITCH - 1),      keyFrame.values.at((uchar)JointId::R_HIP_PITCH - 1));
      std::swap(keyFrame.values.at((uchar)JointId::L_HIP_ROLL - 1),       keyFrame.values.at((uchar)JointId::R_HIP_ROLL - 1));

      // Mirror values
      for (uchar jointId = (uchar)JointId::R_SHOULDER_PITCH; jointId <= (uchar)JointId::HEAD_PAN; jointId++)
        keyFrame.values.at(jointId - (uchar)1) = MX28::getMirrorValue(keyFrame.values.at(jointId - (uchar)1));
    }
  }

  return mirror;
}
