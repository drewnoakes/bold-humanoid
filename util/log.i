%{
#include <util/log.hh>
%}

namespace bold
{
  class log
  {
  public:
  };

  %extend log {
    static void setMinLevelVerbose() { bold::log::minLevel = bold::LogLevel::Verbose; }
    static void setMinLevelInfo() { bold::log::minLevel = bold::LogLevel::Info; }
    static void setMinLevelWarning() { bold::log::minLevel = bold::LogLevel::Warning; }
    static void setMinLevelError() { bold::log::minLevel = bold::LogLevel::Error; }

    static void writeVerbose(std::string const& str) { bold::log::verbose("BoldPy") << str; }
    static void writeInfo(std::string const& str) { bold::log::info("BoldPy") << str; }
    static void writeWarning(std::string const& str) { bold::log::warning("BoldPy") << str; }
    static void writeError(std::string const& str) { bold::log::error("BoldPy") << str; }

    static void writeVerbose(std::string const& scope, std::string const& str)
    {
      bold::log::verbose(std::string("BoldPy.") + scope) << str;
    }
    static void writeInfo(std::string const& scope, std::string const& str)
    {
      bold::log::info(std::string("BoldPy.") + scope) << str;
    }
    static void writeWarning(std::string const& scope, std::string const& str)
    {
      bold::log::warning(std::string("BoldPy.") + scope) << str;
    }
    static void writeError(std::string const& scope, std::string const& str)
    {
      bold::log::error(std::string("BoldPy.") + scope) << str;
    }
  }
}
