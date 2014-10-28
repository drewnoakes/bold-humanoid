#include <gtest/gtest.h>
#include "../Agent/agent.hh"

#include "../Config/config.hh"
#include "../State/state.hh"
#include "../FieldMap/fieldmap.hh"

using namespace bold;

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

  Config::setPermissible();

  FieldMap::initialise();

  return RUN_ALL_TESTS();
}
