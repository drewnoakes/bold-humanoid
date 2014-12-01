#pragma once

#include "../stateobject.hh"

#include <Eigen/Geometry>

namespace bold
{
  class OdometryState : public StateObject
  {
  public:
    OdometryState(Eigen::Affine3d transform)
      : d_transform{std::move(transform)}
    {}

    /** Gets the cumulative transform of the agent frame
     *
     * The returned transformation describes the agent frame at t = 0
     * (A0) wrt the current frame (At). i.e. AtA0, transforming a
     * vector v in A0 by AtA0 * v0 gives that vector in the current
     * agent frame.
     *
     * Users of this value can compute their own delta values through
     * $\delta = A_tA_{t-1} = A_tA_0 * A_{t-1}A0^{-1}$
     */
    Eigen::Affine3d const& getTransform() const { return d_transform; };

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    Eigen::Affine3d d_transform;
  };

  template<typename TBuffer>
  inline void OdometryState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
  {
    writer.StartObject();
    {
      writer.String("tr");
      writer.StartArray();
      for (unsigned j = 0; j < 4; ++j)
      {
        for (unsigned i = 0; i < 4; ++i)
          writer.Double(d_transform.matrix()(i, j), "%.3f");
        writer.String(" ");
      }
      writer.EndArray();
    }
    writer.EndObject();
  }
}
