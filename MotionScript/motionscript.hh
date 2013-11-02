#pragma once

#include "../JointId/jointid.hh"

#include <rapidjson/writer.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>

#include <string>
#include <vector>
#include <memory>

namespace bold
{
  typedef unsigned char uchar;
  typedef unsigned short ushort;

  class MotionScript
  {
  public:
    static const ushort INVALID_BIT_MASK = 0x4000;

    struct KeyFrame
    {
      KeyFrame()
      : pauseCycles(0),
        moveCycles(0),
        values()
      {}

      ushort getValue(uchar jointId) const { return values[jointId-1]; }

      uchar pauseCycles;
      uchar moveCycles;
      ushort values[(uchar)JointId::MAX];
    };

    struct Stage
    {
      static const uchar DEFAULT_SPEED = 32;
      static const uchar DEFAULT_P_GAIN = 32;

      Stage()
      : speed(DEFAULT_SPEED),
        repeatCount(1),
        keyFrames()
      {
        memset(&pGains, DEFAULT_P_GAIN, sizeof(pGains));
      }

      inline uchar getPGain(uchar jointId) const { return pGains[jointId - 1]; }

      unsigned speed;
      unsigned repeatCount;
      uchar pGains[(uchar)JointId::MAX];
      std::vector<KeyFrame> keyFrames;
    };

    static std::shared_ptr<MotionScript> fromFile(std::string fileName);

    static std::vector<std::shared_ptr<MotionScript>> loadAllInPath(std::string path);

    MotionScript(std::string name, std::vector<std::shared_ptr<Stage>> stages)
    : d_name(name),
      d_stages(stages)
    {}

    std::string getName() const { return d_name; }
    void setName(std::string name) { d_name = name; }

    bool writeJsonFile(std::string fileName) const;
    void writeJson(rapidjson::PrettyWriter<rapidjson::FileWriteStream>& writer) const;

    int getStageCount() const { return d_stages.size(); }
    std::shared_ptr<Stage const> getStage(unsigned index) const { return d_stages[index]; }

  private:
    std::string d_name;
    std::vector<std::shared_ptr<Stage>> d_stages;
  };
}
