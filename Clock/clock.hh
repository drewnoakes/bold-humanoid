#ifndef BOLD_CLOCK_HH
#define BOLD_CLOCK_HH

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

#endif

