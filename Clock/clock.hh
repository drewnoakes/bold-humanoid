#pragma once

namespace bold
{
  class Clock
  {
  public:
    typedef unsigned long long Timestamp;

    static Timestamp getTimestamp();

    static double getMillis();

    static double getSeconds();

    static double getSecondsSince(Timestamp since);

    static double getMillisSince(Timestamp since);

    static double timestampToMillis(Timestamp timestamp);

    static double timestampToSeconds(Timestamp timestamp);

  private:
    Clock() = delete;
  };
}
