#include "gtest/gtest.h"

#include <iostream>
#include <Eigen/Core>
#include <Eigen/Geometry>

#include "../Agent/agent.hh"
#include "../Config/config.hh"
#include "../State/state.hh"
#include "../util/log.hh"

using namespace Eigen;
using namespace bold;

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

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  log::minLevel = LogLevel::Warning;

  for (int i = 0; i < argc; i++)
  {
    if (strcmp("-v", argv[i]) == 0)
      log::minLevel = LogLevel::Verbose;
  }

  State::initialise();

  Agent::registerStateTypes();

  Config::initialise("../configuration-metadata.json", "../configuration-team.json");

  return RUN_ALL_TESTS();
}
