#pragma once

#include "../stateobject.hh"
#include "../../JsonWriter/jsonwriter.hh"

#include <Eigen/Geometry>

namespace bold
{
  class OrientationState : public StateObject
  {
  public:
    OrientationState(Eigen::Quaterniond const& quaternion);

    Eigen::Quaterniond const& getQuaternion() const { return d_quaternion; };

    double getPitchAngle() const { return d_pitch; }
    double getRollAngle() const { return d_roll; }
    double getYawAngle() const { return d_yaw; }

    Eigen::Affine3d withoutYaw() const;

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    Eigen::Quaterniond d_quaternion;
    double d_pitch;
    double d_roll;
    double d_yaw;
  };

  template<typename TBuffer>
  inline void OrientationState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
  {
    writer.StartObject();
    {
      writer.String("quaternion");
      writer.StartArray();
      JsonWriter::swapNaN(writer, d_quaternion.x(), "%.6f");
      JsonWriter::swapNaN(writer, d_quaternion.y(), "%.6f");
      JsonWriter::swapNaN(writer, d_quaternion.z(), "%.6f");
      JsonWriter::swapNaN(writer, d_quaternion.w(), "%.6f");
      writer.EndArray();

      writer.String("pitch"); JsonWriter::swapNaN(writer, d_pitch, "%.3f");
      writer.String("roll");  JsonWriter::swapNaN(writer, d_roll, "%.3f");
      writer.String("yaw");   JsonWriter::swapNaN(writer, d_yaw, "%.3f");
    }
    writer.EndObject();
  }
}
