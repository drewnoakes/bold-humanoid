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
    
    static double timestampToMillis(Timestamp timestamp) { return timestamp / 1e3; }
    
    static double timestampToSeconds(Timestamp timestamp) { return timestamp / 1e6; }

  private:
    Clock() {}
  };
}
