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

bool operator==(Vector2i const& a, Vector2i const& b)
{
  return a.x() == b.x() && a.y() == b.y();
}

#include "AgentModelTests.hh"
#include "Bounds2iTests.hh"
#include "CameraModelTests.hh"
#include "ColourTests.hh"
#include "DistributionTrackerTests.hh"
#include "LineSegmentTests.hh"
#include "RunTests.hh"

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
