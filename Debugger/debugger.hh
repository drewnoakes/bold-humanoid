#ifndef BOLD_DEBUGGER_HH
#define BOLD_DEBUGGER_HH

#include <vector>

#include <LinuxDARwIn.h>
#include <LinuxCM730.h>

#include "../GameController/RoboCupGameControlData.h"
#include "../DataStreamer/datastreamer.hh"
#include <BlobDetector/blobdetector.hh>

#define LED_RED   0x01;
#define LED_BLUE  0x02;
#define LED_GREEN 0x04;

namespace bold
{
//   struct EventTiming
//   {
//     EventTiming()
//     : timeSeconds(0), eventName("")
//     {}
//
//     EventTiming(double timeSeconds, std::string const& eventName)
//     : timeSeconds(timeSeconds), eventName(eventName)
//     {}
//
//     double const timeSeconds;
//     std::string const& eventName;
//   };

  typedef std::pair<double,std::string> EventTiming;

  class Debugger
  {
  public:
    typedef unsigned long long timestamp_t;

  private:
    Debugger();
    int d_lastLedFlags;
    std::vector<EventTiming> d_eventTimings;

  public:
    static const timestamp_t getTimestamp();

    static const double getSeconds(timestamp_t const& startedAt);

    // TODO review usages of this, and exchange for timeEvent
    static const double printTime(timestamp_t const& startedAt, std::string const& description);

    // TODO return something that tracks nested events, using block-based destruction
    timestamp_t timeEvent(timestamp_t const& startedAt, std::string const& eventName);

    void addEventTiming(EventTiming const& eventTiming);

    std::vector<EventTiming> getTimings()
    {
      return d_eventTimings;
    }

    void clearTimings()
    {
      d_eventTimings = std::vector<EventTiming>();
    }

//     void processBlobs(std::vector<std::set<Blob>> const& blobs)
//     {
//       if (!d_showUI)
//         return;
//
//       // Draw rectangles on the colour image
//       for (set<Blob>& blobSet : blobs)
//       {
//         for (Blob const& b : blobSet)
//         {
//           if ((b.br - b.ul).minCoeff() > 5)
//             cv::rectangle(image,
//                           cv::Rect(b.ul.x(), b.ul.y(),
//                                   b.br.x() - b.ul.x(), b.br.y() - b.ul.y()),
//                           cv::Scalar(255,0,0),
//                           2);
//         }
//       }
//
//       for (Observation const& obs : observations)
//       {
//         cv::Scalar color;
//         switch (obs.type)
//         {
//           case O_BALL:      color = cv::Scalar(0,0,255);   break;
//           case O_GOAL_POST: color = cv::Scalar(0,255,255); break;
//         }
//         cv::circle(image, cv::Point(obs.pos.x(), obs.pos.y()), 5, color, 2);
//       }
//
//       cv::imshow("raw", image);
//       cv::normalize(labelled, labelled, 0, 255, CV_MINMAX);
//       cv::imshow("labelled", labelled);
//
//       cv::waitKey(1);
//     }

    /**
     * Update the debugger at the end of the think cycle.
     * Currently only updates LEDs.
     */
    void update(Robot::CM730& cm730);

    static Debugger& getInstance()
    {
      static Debugger instance;
      return instance;
    }
  };
}

#endif
