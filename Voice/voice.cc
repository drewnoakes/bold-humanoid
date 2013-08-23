#include "voice.hh"

#include <espeak/speak_lib.h>
#include <iostream>
#include <future>

using namespace bold;
using namespace std;

Voice::Voice(const string voice)
: d_voiceName(voice)
{
  espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, 500, nullptr, 0);
  espeak_SetVoiceByName(d_voiceName.c_str());
}

void Voice::say(const string message)
{
  cout << "[Voice::say] Saying: " << message << endl;
  std::async(launch::async, [message]()
             {
               espeak_Synth(message.c_str(), message.length(), 0, POS_CHARACTER, 0, espeakCHARS_AUTO, nullptr, nullptr);
               espeak_Synchronize();
             });
}
