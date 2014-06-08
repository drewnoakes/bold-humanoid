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

      KeyFrame(KeyFrame const& other) = default;

      /// Gets the position value to apply to the specified joint during this stage of motion
      ushort getValue(uchar jointId) const { return values.at(jointId - 1u); }

      /// The number of cycles to pause for after the target position has been reached.
      uchar pauseCycles;
      /// The number of cycles over which motion towards the target position is planned.
      uchar moveCycles;
      /// The target motor position values, indexed by joint ID - 1.
      std::array<ushort,(int)JointId::MAX> values;
    };

    /** Describes a group of key frames that share some configuration, and may be repeated several times.
     *
     * Configuration options are speed and p-gains per joint.
     */
    struct Stage
    {
      static const uchar DEFAULT_SPEED;
      static const uchar DEFAULT_P_GAIN;

      Stage()
      : speed(DEFAULT_SPEED),
        repeatCount(1),
        keyFrames()
      {
        pGains.fill(DEFAULT_P_GAIN);
      }

      Stage(Stage const& other) = default;

      /// Gets the proportional gain to apply to the specified joint during this stage of motion
      inline uchar getPGain(uchar jointId) const { return pGains.at(jointId - 1u); }

      /// The speed for motion during this stage. Defaults to DEFAULT_SPEED.
      unsigned speed;
      /// The number of times this stage should be played. Defaults to 1.
      unsigned repeatCount;
      /// The p-gain value to be used throughout this stage, indexed by joint ID - 1.
      std::array<uchar,(int)JointId::MAX> pGains;
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
    MotionScript(std::string name, std::vector<std::shared_ptr<Stage>> stages,
                 bool controlsHead, bool controlsArms, bool controlsLegs)
    : d_name(name),
      d_stages(stages),
      d_controlsHead(controlsHead),
      d_controlsArms(controlsArms),
      d_controlsLegs(controlsLegs)
    {}

    std::string getName() const { return d_name; }
    void setName(std::string name) { d_name = name; }

    bool writeJsonFile(std::string fileName) const;
    void writeJson(rapidjson::PrettyWriter<rapidjson::FileWriteStream>& writer) const;

    int getStageCount() const { return d_stages.size(); }
    std::shared_ptr<Stage const> getStage(unsigned index) const { return d_stages[index]; }
    std::shared_ptr<Stage const> getFirstStage() const;
    std::shared_ptr<Stage const> getFinalStage() const;
    KeyFrame const& getFinalKeyFrame() const;

    bool getControlsHead() const { return d_controlsHead; }
    bool getControlsArms() const { return d_controlsArms; }
    bool getControlsLegs() const { return d_controlsLegs; }

  private:
    std::string d_name;
    std::vector<std::shared_ptr<Stage>> d_stages;
    bool d_controlsHead;
    bool d_controlsArms;
    bool d_controlsLegs;
  };
}
