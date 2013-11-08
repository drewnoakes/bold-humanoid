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

  /** Describes a playable motion sequence defined using key frames.
   */
  class MotionScript
  {
  public:
    static const ushort INVALID_BIT_MASK = 0x4000;

    /** Describes a body posture via a set of target motor positions.
     *
     * Also defines the number of cycles over which to approach this target position.
     * An optional number of cycles to pause for after motion completes may be specified.
     * Therefore, total duration of this keyframe is moveCycles + pauseCycles.
     */
    struct KeyFrame
    {
      KeyFrame()
      : pauseCycles(0),
        moveCycles(0),
        values()
      {}

      /// Gets the position value to apply to the specified joint during this stage of motion
      ushort getValue(uchar jointId) const { return values[jointId - 1]; }

      /// The number of cycles to pause for after the target position has been reached.
      uchar pauseCycles;
      /// The number of cycles over which motion towards the target position is planned.
      uchar moveCycles;
      /// The target motor position values, indexed by joint ID.
      ushort values[(uchar)JointId::MAX];
    };

    /** Describes a group of key frames that share some configuration, and may be repeated several times.
     *
     * Configuration options are speed and p-gains per joint.
     */
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

      /// Gets the proportional gain to apply to the specified joint during this stage of motion
      inline uchar getPGain(uchar jointId) const { return pGains[jointId - 1]; }

      /// The speed for motion during this stage. Defaults to DEFAULT_SPEED.
      unsigned speed;
      /// The number of times this stage should be played. Defaults to 1.
      unsigned repeatCount;
      /// The p-gain value to be used throught this stage, indexed by joint ID.
      uchar pGains[(uchar)JointId::MAX];
      /// The KeyFrames contained within this stage.
      std::vector<KeyFrame> keyFrames;
    };

    /** Loads a MotionScript from the specified JSON file.
     */
    static std::shared_ptr<MotionScript> fromFile(std::string fileName);

    /** Loads all MotionScript files found in the specified path.
     */
    static std::vector<std::shared_ptr<MotionScript>> loadAllInPath(std::string path);

    /** Initialises the MotionScript with specified name and stages.
     */
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
