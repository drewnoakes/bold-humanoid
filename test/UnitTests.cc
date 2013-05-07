#include "gtest/gtest.h"

#include <iostream>
#include <Eigen/Core>
#include <Eigen/Geometry>

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

std::ostream& operator<<(std::ostream& stream, Vector3d const& v)
{
  return stream << "(" << v.x() << ", " << v.y() << ", " << v.z() << ")";
}

bool operator==(Vector2i const& a, Vector2i const& b)
{
  return a.x() == b.x() && a.y() == b.y();
}

#include "AgentPositionTests.hh"
#include "BodyStateTests.hh"
#include "BlobTests.hh"
#include "Bounds2iTests.hh"
#include "CameraModelTests.hh"
#include "ColourTests.hh"
#include "DistributionTrackerTests.hh"
#include "LineSegmentTests.hh"
#include "MathTests.hh"
#include "MovingAverageTests.hh"
#include "ParticleFilterTests.hh"
#include "RobotisTests.hh"
#include "RunTests.hh"
#include "SpatialiserTest.hh"

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
