#include "voice.hh"

#include <espeak/speak_lib.h>
#include <string>
#include <iostream>

#include "../Config/config.hh"
#include "../util/log.hh"

using namespace bold;
using namespace std;

static const std::map<int,std::string const> voiceById = {
  { 1, "english-mb-en1" },
  { 2, "english" },
  { 3, "en-scottish" },
  { 4, "english-north" },
  { 5, "english_rp" },
  { 6, "english_wmids" },
  { 7, "english-us" },
  { 8, "german" },
  { 9, "dutch" }
};

Voice::Voice()
: d_queue(std::bind(&Voice::sayCallback, this, std::placeholders::_1)),
  d_name(Config::getSetting<int>("voice.name")),
  d_rate(Config::getSetting<int>("voice.rate")),
  d_volume(Config::getSetting<int>("voice.volume")),
  d_pitch(Config::getSetting<int>("voice.pitch")),
  d_range(Config::getSetting<int>("voice.range")),
  d_wordGapMs(Config::getSetting<int>("voice.word-gap-ms")),
  d_capitalAccentHz(Config::getSetting<int>("voice.capital-accent-hz")),
  d_initialised(false)
{
/*
  d_voice->sayOneOf({
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
  });
*/

  static const string sayings[] = {
    "Hello", "Bold Hearts", "Hooray", "Oh, my",
    "The rain in spain falls mainly in the plain",
    "Use those hands, Gareth", "Good Hands Gareth",
    "Adventure on Ywain", "Ywain, you bastard",
    "Kick it, Tor",
    "Nim-way, you're some kind of lady",
    "Let's hear it for Oberon"
  };

  int sayingIndex = 1;
  for (auto const& saying : sayings)
  {
    stringstream id;
    id << "voice.speak.saying-" << sayingIndex++;
    Config::addAction(id.str(), saying, [this,saying](){ say(saying); });
  }
}

Voice::~Voice()
{
  if (d_initialised)
    espeak_Terminate();
}

unsigned int Voice::queueLength() const
{
  return d_queue.size();
}

void Voice::say(string const& message)
{
  SpeechTask task = { message, (uint)d_rate->getValue(), true };
  say(task);
}

void Voice::say(SpeechTask const& task)
{
  log::verbose("Voice::say") << "Enqueuing: " << task.message;
  d_queue.push(task);
}

void Voice::sayOneOf(initializer_list<string> const& messages)
{
  size_t index = rand() % messages.size();
  string message = *(messages.begin() + index);
  say(message);
}

void Voice::sayCallback(SpeechTask task)
{
  if (!Config::getValue<bool>("voice.enabled"))
    return;

  if (!d_initialised)
  {
    espeak_Initialize(
      AUDIO_OUTPUT_SYNCH_PLAYBACK, // plays audio data asynchronously
      500,                         // length of buffers for synth function, in ms
      nullptr,                     // dir containing espeak-data, null for default
      0);                          // options are mostly for phoneme callbacks, so 0

    const char* path;
    auto version = espeak_Info(&path);
    log::verbose("Voice::Voice") << "espeak " << version << "(" << path << ")";

    espeak_SetParameter(espeakPUNCTUATION, espeakPUNCT_NONE, 0);

    d_initialised = true;
  }

  int voiceId = d_name->getValue();
  auto const it = voiceById.find(voiceId);
  if (it != voiceById.end())
    espeak_SetVoiceByName(it->second.c_str());

  // Set parameters each time, in case they change
  espeak_SetParameter(espeakRATE,     task.rate,                     0);
  espeak_SetParameter(espeakVOLUME,   d_volume->getValue(),          0);
  espeak_SetParameter(espeakPITCH,    d_pitch->getValue(),           0);
  espeak_SetParameter(espeakRANGE,    d_range->getValue(),           0);
  espeak_SetParameter(espeakWORDGAP,  d_wordGapMs->getValue() / 10,  0); // units of 10ms
  espeak_SetParameter(espeakCAPITALS, d_capitalAccentHz->getValue(), 0);

  auto const& message = task.message;

  log::verbose("Voice") << "Saying: " << message;

  uint flags = espeakCHARS_AUTO; //  AUTO      8 bit or UTF8 automatically
                                 //
  if (task.pauseAfter)           //
    flags |= espeakENDPAUSE;     // ENDPAUSE  sentence pause at end of text

  espeak_ERROR err = espeak_Synth(
    message.c_str(),   // text
    message.length(),  // size
    0,                 // position to start from
    POS_CHARACTER,     // whether above 0 pos is chars/word/sentences
    message.size(),    // end position, 0 indicating no end
    flags,             // as above
    nullptr,           // message identifier given to callback (unused)
    nullptr);          // user data, passed to the callback function (unused)

  if (err != EE_OK)
    log::error("Voice::sayCallback") << "Error synthesising speech";

  log::verbose("Voice::sayCallback") << "Message completed";
}

void Voice::stop()
{
  d_queue.stop();
}
