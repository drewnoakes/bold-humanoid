#pragma once

#include "./Clock/clock.hh"

#include <string>

namespace bold
{
  struct Version
  {
    static const std::string GIT_SHA1;
    static const std::string GIT_DATE;
    static const std::string GIT_COMMIT_SUBJECT;

    /// CMake build type, such as Release, RelWithDebInfo, Debug
    static const std::string BUILD_TYPE;

    /// Host name of thee machine that built this executable
    static const std::string BUILT_ON_HOST_NAME;

    static std::string describeTimeSinceGitDate();
  };
}
