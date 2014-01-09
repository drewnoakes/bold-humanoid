#pragma once

#include <string>

namespace bold
{
  struct Version
  {
    static const std::string GIT_SHA1;
    static const std::string GIT_DATE;
    static const std::string GIT_COMMIT_SUBJECT;

    static std::string describeTimeSinceGitDate();
  };
}
