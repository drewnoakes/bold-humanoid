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
    
    static double timeStampToMillis(Timestamp timestamp) { return timestamp / 1e3; }
    
    static double timeStampToSeconds(Timestamp timestamp) { return timestamp / 1e6; }

  private:
    Clock() {}
  };
}
