#include "voice.hh"

#include <espeak/speak_lib.h>
#include <string>
#include <iostream>

#include "../Config/config.hh"
#include "../util/log.hh"

using namespace bold;
using namespace std;

Voice::Voice(const string voice)
: d_voiceName(voice),
  d_queue(std::bind(&Voice::sayCallback, this, std::placeholders::_1)),
  d_initialised(false)
{
  // TODO SETTINGS specify these strings in configuration

/*
  vector<string> phrases = {
    "Bold Hearts are go",
//     "I am a protector of the realm",
//     "What do you despise? By this are you truly known.",
//     "A day may come when the courage of men fails",
//     "Duty is heavier than a mountain",
//     "Humans have a knack for choosing precisely the things that are worst for them",
//     "Ride for ruin and the world's ending!",
//     "Kill if you will, but command me nothing!",
//     "The existence of tricks does not imply the absence of magic",
//     "We eat ham and jam and Spam a lot"
  };
  d_voice->say(phrases[rand() % phrases.size()]);
*/

  string sayings[] = {
    "Hello", "Bold Hearts", "Hooray", "Oh my",
    "The rain in spain falls mainly in the plain",
    "Use those hands, Gareth", "Good Hands Gareth",
    "Adventure on Ywain", "Ywain, you bastard",
    "Kick it, Tor",
    "Nimue, you're some kind of lady",
    "Let's hear it for Oberon"
  };

  int sayingIndex = 1;
  for (auto saying : sayings)
  {
    stringstream id;
    id << "voice.speak.saying-" << sayingIndex++;
    Config::addAction(id.str(), saying, [this,saying](){ say(saying); });
  }
}

void Voice::say(string message)
{
  log::verbose("Voice::say") << "Enqueuing: " << message;
  d_queue.push(message);
}

void Voice::sayOneOf(initializer_list<string> messages)
{
  size_t index = rand() % messages.size();
  string message = *(messages.begin() + index);
  say(message);
}

void Voice::sayCallback(string message)
{
  if (!d_initialised)
  {
    espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, 500, nullptr, 0);
    espeak_SetVoiceByName(d_voiceName.c_str());
    d_initialised = true;
  }

  log::verbose("Voice::sayCallback") << "Saying: " << message;
  espeak_Synth(message.c_str(), message.length(), 0, POS_CHARACTER, 0, espeakCHARS_AUTO, nullptr, nullptr);
  espeak_Synchronize();

  // Leave a while between phrases, so the speech doesn't become garbled.
  log::verbose("Voice::sayCallback") << "**** Sleeping " << message;
  this_thread::sleep_for(chrono::seconds(1));
  log::verbose("Voice::sayCallback") << "**** Done " << message;
}

void Voice::stop()
{
  d_queue.stop();
}
