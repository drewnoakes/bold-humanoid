#pragma once

#include "../stateobject.hh"
#include "../../MotionModule/WalkModule/walkmodule.hh"

namespace bold
{
  class WalkEngine;

  class WalkState : public StateObject
  {
  public:
    WalkState(double targetX, double targetY, double targetTurn, double targetHipPitch,
              double lastXDelta, double lastYDelta, double lastTurnDelta, double lastHipPitchDelta,
              WalkModule* walkModule,
              std::shared_ptr<WalkEngine> walkEngine);

    // From walk module

    bool isRunning() const { return d_isRunning; }
    WalkStatus getStatus() const { return d_status; }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    // From walk module
    bool d_isRunning;
    WalkStatus d_status;

    // From walk engine
    double d_targetX;
    double d_targetY;
    double d_targetTurn;
    double d_targetHipPitch;
    double d_currentX;
    double d_currentY;
    double d_currentTurn;
    double d_currentHipPitch;
    double d_lastXDelta;
    double d_lastYDelta;
    double d_lastTurnDelta;
    double d_lastHipPitchDelta;
    int d_currentPhase;
    double d_bodySwingY;
    double d_bodySwingZ;
  };

  template<typename TBuffer>
  inline void WalkState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
  {
    writer.StartObject();
    {
      writer.String("running");
      writer.Bool(d_isRunning);
      writer.String("status");
      writer.Int((int)d_status);

      writer.String("target");
      writer.StartArray();
      {
        writer.Double(d_targetX, "%.2f");
        writer.Double(d_targetY, "%.2f");
        writer.Double(d_targetTurn, "%.2f");
      }
      writer.EndArray();

      writer.String("current");
      writer.StartArray();
      {
        writer.Double(d_currentX, "%.2f");
        writer.Double(d_currentY, "%.2f");
        writer.Double(d_currentTurn, "%.2f");
        writer.Double(d_currentHipPitch, "%.2f");
      }
      writer.EndArray();

      writer.String("delta");
      writer.StartArray();
      {
        writer.Double(d_lastXDelta, "%.2f");
        writer.Double(d_lastYDelta, "%.2f");
        writer.Double(d_lastTurnDelta, "%.2f");
        writer.Double(d_lastHipPitchDelta, "%.2f");
      }
      writer.EndArray();

      writer.String("phase");
      writer.Int(d_currentPhase);

      writer.String("hipPitch");
      writer.StartObject();
      writer.String("target");
      writer.Double(d_targetHipPitch, "%.3f");
      writer.String("current");
      writer.Double(d_currentHipPitch, "%.3f");
      writer.String("delta");
      writer.Double(d_lastHipPitchDelta, "%.3f");
      writer.EndObject();

      writer.String("bodySwingY");
      writer.Double(d_bodySwingY, "%.3f");
      writer.String("bodySwingZ");
      writer.Double(d_bodySwingZ, "%.3f");
    }
    writer.EndObject();
  }
}
