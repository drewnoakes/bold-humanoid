#pragma once

#include "../stateobject.hh"

#include <memory>
#include <vector>
#include <Eigen/Core>

namespace bold
{
  /** Holds a set of ParticleFilter particles at a given point in time.
   */
  class ParticleState : public StateObject
  {
  public:
    ParticleState(Eigen::MatrixXd const& particles, double preNormWeightSum, double smoothedPreNormWeightSum, double uncertainty)
      : d_particles{particles},
      d_preNormWeightSum{preNormWeightSum},
      d_smoothedPreNormWeightSum{smoothedPreNormWeightSum},
      d_uncertainty{uncertainty}
    {}

    Eigen::MatrixXd const& getParticles() const { return d_particles; }
    double getPreNormWeightSum() const { return d_preNormWeightSum; }
    double getSmoothedPreNormWeightSum() const { return d_smoothedPreNormWeightSum; }
    double getUncertainty() const { return d_uncertainty; }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    Eigen::MatrixXd d_particles;
    double d_preNormWeightSum;
    double d_smoothedPreNormWeightSum;
    double d_uncertainty;
  };

  template<typename TBuffer>
  void ParticleState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
  {
    writer.StartObject();
    {
      writer.String("particles");
      writer.StartArray();
      {
        for (int i = 0; i < d_particles.cols(); ++i)
        {
          writer.StartArray();

          auto particle = d_particles.col(i);
          writer.Double(particle.x(), "%.3f"); // x
          writer.Double(particle.y(), "%.3f"); // y
          writer.Double(atan2(particle(3), particle(2)), "%.3f"); // theta
          JsonWriter::swapNaN(writer, particle(4), "%.4f"); // weight

          writer.EndArray();
        }
      }
      writer.EndArray();

      writer.String("pnwsum");
      writer.Double(d_preNormWeightSum, "%.3f");
      writer.String("pnwsumsmooth");
      writer.Double(d_smoothedPreNormWeightSum, "%.3f");
      writer.String("uncertainty");
      writer.Double(d_uncertainty, "%.3f");
    }
    writer.EndObject();
  }
}
