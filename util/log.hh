#pragma once

#include <iostream>
#include <sstream>
#include <memory>
#include <vector>
#include "memory.hh"

namespace bold
{
  enum class LogLevel
  {
    Trace = 0,
    Verbose,
    Info,
    Warning,
    Error
  };

  class LogAppender
  {
  public:
    virtual void append(LogLevel level, std::string const& scope, std::string const& message) = 0;
  };

  /** A very minimal logging framework to control console output.
   */
  class log
  {
  private:
    log(LogLevel level)
    : log(std::string(), level)
    {}

    log(std::string const& scope, LogLevel level)
    : d_scope(scope),
      d_level(level),
      d_message(level < minLevel ? nullptr : new std::ostringstream())
    {}

  public:
    static LogLevel minLevel;
    static bool logGameState;

    template<typename T,typename... Args>
    static std::shared_ptr<T> addAppender(Args&&... args)
    {
      static_assert(std::is_base_of<LogAppender,T>::value, "Type must implement LogAppender");
      const std::shared_ptr<T> ptr = std::make_shared<T>(std::forward<Args>(args)...);
      d_appenders.push_back(ptr);
      return ptr;
    }

    static log trace()   { return log(LogLevel::Trace); }
    static log info()    { return log(LogLevel::Info); }
    static log verbose() { return log(LogLevel::Verbose); }
    static log warning() { return log(LogLevel::Warning); }
    static log error()   { return log(LogLevel::Error); }

    static log trace(std::string const& scope)   { return log(scope, LogLevel::Trace); }
    static log info(std::string const& scope)    { return log(scope, LogLevel::Info); }
    static log verbose(std::string const& scope) { return log(scope, LogLevel::Verbose); }
    static log warning(std::string const& scope) { return log(scope, LogLevel::Warning); }
    static log error(std::string const& scope)   { return log(scope, LogLevel::Error); }

    log(log&& log)
    : d_scope(log.d_scope),
      d_level(log.d_level),
      d_message(std::move(log.d_message))
    {}

    ~log();

    template<typename T>
    log& operator<<(T const& value)
    {
      if (d_message)
        *d_message << value;
      return *this;
    }

  private:
    static std::vector<std::shared_ptr<LogAppender>> d_appenders;
    std::string d_scope;
    LogLevel d_level;
    std::unique_ptr<std::ostringstream> d_message;
  };

  class ConsoleLogAppender : public LogAppender
  {
  public:
    void append(LogLevel level, std::string const& scope, std::string const& message) override;

    static bool isStdOutRedirected();
    static bool isStdErrRedirected();
  };

  inline std::ostream& operator<<(std::ostream &stream, std::vector<std::string> const& strings)
  {
    stream <<  "[";
    bool comma = false;
    for (auto const& s : strings)
    {
      if (comma)
        stream << ",";
      comma = true;
      stream << "\"" << s << "\"";
    }
    stream <<  "]";
    return stream;
  }
}
