#ifndef BOLD_DEBUGGER_HH
#define BOLD_DEBUGGER_HH

#define LED_RED  0x01;
#define LED_BLUE 0x02;

namespace bold
{
  class Debugger
  {
  public:
    typedef unsigned long long timestamp_t;

  private:
    bool d_isBallObserved;
    bool d_isImageProcessingSlow; 
    int d_lastLEDValue;
    double d_imageProcessingThresholdMillis;

    double getSeconds(timestamp_t startedAt)
    {
      timestamp_t now = getTimestamp();
      return (now - startedAt) / 1000000.0L;
    }
  
  public:
    Debugger()
    : d_isBallObserved(false),
      d_isImageProcessingSlow(false),
      d_lastLEDValue(0xff),
      d_imageProcessingThresholdMillis(25.0)
    {}

    static timestamp_t getTimestamp()
    {
      struct timeval now;
      gettimeofday(&now, NULL);
      return now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
    }

    void timeImageProcessing(timestamp_t startedAt)
    {
      double millis = getSeconds(startedAt) * 1000.0;
      bool isOverThreshold = millis > d_imageProcessingThresholdMillis;
      fprintf(isOverThreshold ? stderr : stdout, "Image processed in %.1fms\n", millis);
      d_isImageProcessingSlow = isOverThreshold;
    }

    void setIsBallObserved(bool isBallObserved)
    {
      d_isBallObserved = isBallObserved;
    }
    
    void update(Robot::CM730& cm730)
    {
      int value = 0;
      if (d_isBallObserved)
        value |= LED_RED;
      if (d_isImageProcessingSlow)
        value |= LED_BLUE;
      if (value != d_lastLEDValue)
      {
        cm730.WriteByte(Robot::CM730::P_LED_PANNEL, value, NULL);
        d_lastLEDValue = value;
      }
    }
  };
}

#endif
