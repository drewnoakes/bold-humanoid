#pragma once

#include <initializer_list>
#include <string>

#include "../util/consumerqueuethread.hh"

namespace bold
{
  typedef unsigned int uint;

  template<typename> class Setting;

  struct SpeechTask
  {
    std::string message;
    uint rate;
    bool pauseAfter;
  };

  class Voice
  {
  public:
    Voice();
    ~Voice();

    void say(std::string const& message);
    void say(SpeechTask const& task);

    void sayOneOf(std::initializer_list<std::string> const& messages);

    void stop();

    unsigned queueLength() const;

  private:
    Voice(const Voice&) = delete;
    Voice& operator=(const Voice&) = delete;

    void sayCallback(SpeechTask message);

    ConsumerQueueThread<SpeechTask> d_queue;
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
