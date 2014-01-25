%{
#include <util/log.hh>
%}

namespace bold
{
  class LogLevel;

  class log
  {
  public:
    
  };

  %extend log {
    static void setMinLevelVerbose() { bold::log::minLevel = bold::LogLevel::Verbose; }
    static void setMinLevelInfo() { bold::log::minLevel = bold::LogLevel::Info; }
    static void setMinLevelWarning() { bold::log::minLevel = bold::LogLevel::Warning; }
    static void setMinLevelError() { bold::log::minLevel = bold::LogLevel::Error; }
  }
}
