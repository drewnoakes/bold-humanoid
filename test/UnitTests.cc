#include "gtest/gtest.h"

#include <iostream>
#include <Eigen/Core>
#include <Eigen/Geometry>

#include "../util/log.hh"

using namespace Eigen;

//PrintTo(const T&, ostream*)

std::ostream& operator<<(std::ostream& stream, Vector2i const& v)
{
  return stream << "(" << v.x() << ", " << v.y() << ")";
}

std::ostream& operator<<(std::ostream& stream, Vector2f const& v)
{
  return stream << "(" << v.x() << ", " << v.y() << ")";
}

std::ostream& operator<<(std::ostream& stream, Vector2d const& v)
{
  return stream << "(" << v.x() << ", " << v.y() << ")";
}

std::ostream& operator<<(std::ostream& stream, Vector3d const& v)
{
  return stream << "(" << v.x() << ", " << v.y() << ", " << v.z() << ")";
}

bool operator==(Vector2i const& a, Vector2i const& b)
{
  return a.x() == b.x() && a.y() == b.y();
}

#include "AgentPositionTests.hh"
#include "AgentStateTests.hh"
#include "BodyStateTests.hh"
#include "BlobTests.hh"
#include "Bounds2iTests.hh"
#include "BoundsTests.hh"
#include "CameraModelTests.hh"
#include "CM730Tests.hh"
#include "ConditionalsTests.hh"
#include "CppTests.hh"
#include "ColourTests.hh"
#include "ConsumerQueueThreadTests.hh"
#include "DistributionTrackerTests.hh"
#include "EigenTests.hh"
#include "HalfHullBuilderTests.hh"
#include "IntegralImageTests.hh"
#include "JointSelectionTests.hh"
#include "LinearSmootherTests.hh"
#include "LineSegmentTests.hh"
#include "MathTests.hh"
#include "MetaTests.hh"
#include "MX28AlarmTests.hh"
#include "MX28Tests.hh"
#include "MotionScriptRunnerTests.hh"
#include "ParticleFilterTests.hh"
#include "Polygon2Tests.hh"
#include "RangeTests.hh"
#include "RunTests.hh"
#include "SequentialTimerTests.hh"
#include "SchmittTriggerTests.hh"
#include "SpatialiserTest.hh"
#include "StatsTests.hh"
#include "ThreadTests.hh"
#include "ThreadIdTests.hh"
#include "UDPSocketTests.hh"
#include "VisualCortexTests.hh"

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  log::minLevel = LogLevel::Warning;

  AgentState::initialise();

  Config::initialise("../configuration-metadata.json", "../configuration-team.json");

  for (int i = 0; i < argc; i++)
  {
    if (strcmp("-v", argv[i]) == 0)
      log::minLevel = LogLevel::Verbose;
  }

  return RUN_ALL_TESTS();
}
