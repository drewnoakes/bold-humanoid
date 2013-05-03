#pragma once

namespace bold
{
  class Clock
  {
  public:
    typedef unsigned long long Timestamp;

    static Timestamp getTimestamp();

    static double getSeconds();

    static double getSeconds(Timestamp since);

  private:
    Clock() {}
  };

}
