#pragma once

#include <initializer_list>
#include <string>

#include "../util/consumerqueuethread.hh"

namespace bold
{
  template<class> class Setting;

  class Voice
  {
  public:
    Voice();
    ~Voice();

    void say(std::string message);

    void sayOneOf(std::initializer_list<std::string> messages);

    void stop();

    unsigned queueLength() const;

  private:
    void sayCallback(std::string message);

    ConsumerQueueThread<std::string> d_queue;
    Setting<int>* d_name;
    Setting<int>* d_rate;
    Setting<int>* d_volume;
    Setting<int>* d_pitch;
    Setting<int>* d_range;
    Setting<int>* d_wordGapMs;
    Setting<int>* d_capitalAccentHz;
    bool d_initialised;
  };
}
