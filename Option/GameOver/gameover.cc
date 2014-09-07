#include "gameover.hh"

#include "../MotionScriptOption/motionscriptoption.hh"
#include "../../State/state.hh"
#include "../../StateObject/GameState/gamestate.hh"
#include "../../MotionScript/motionscript.hh"

#include <random>

using namespace bold;
using namespace std;
using namespace rapidjson;

GameOver::GameOver(string const& id, shared_ptr<MotionScriptModule> motionScriptModule, shared_ptr<Voice> voice)
: Option(id, "GameOver"),
  d_motionScriptOption(make_shared<MotionScriptOption>("game-over-motion", motionScriptModule)),
  d_voice(voice),
  d_result(GameResult::Undecided),
  d_isScriptPlaying(false),
  d_playCount(0),
  d_hasTerminated(false)
{}

vector<shared_ptr<Option>> GameOver::runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer)
{
  ASSERT(!d_hasTerminated);

  // If we've already selected a script to run, continue it
  if (d_isScriptPlaying)
  {
    // If it's finished, clear our flag
    if (d_motionScriptOption->hasTerminated())
      d_isScriptPlaying = false;
    else
      return { d_motionScriptOption };
  }

  // If we haven't cached the game result yet, try to do so now.
  // Note that we cache this as the game controller may be turned off at the end of the game
  // and we remove the GameState object after some time. This allows us to be victorious for
  // as long as we like :)
  if (d_result == GameResult::Undecided)
  {
    auto game = State::get<GameState>();

    if (!game)
    {
      log::error("GameOver::runPolicy") << "No game state available";
      return {};
    }

    d_result = game->getGameResult();
    log::info("GameOver::runPolicy") << "Game result determined as " << getGameResultName(d_result);

    if (d_result == GameResult::Undecided)
    {
      d_hasTerminated = true;
      return {};
    }
  }

  auto setRandomScript = [this](initializer_list<string> const& scripts) -> vector<shared_ptr<Option>>
  {
    if (++d_playCount > 3)
    {
      d_hasTerminated = true;
      return {};
    }
    static default_random_engine randomness((uint)time(0));
    size_t index = randomness() % scripts.size();
    string script = *(scripts.begin() + index);
    log::info("GameOver::runPolicy") << "Selecting script " << script;
    d_motionScriptOption->setMotionScript(MotionScript::fromFile(script));
    d_motionScriptOption->reset();
    d_isScriptPlaying = true;
    return { d_motionScriptOption };
  };

  switch (d_result)
  {
    case GameResult::Draw:
    {
      return setRandomScript({"./motionscripts/body-language-no.json"});
    }
    case GameResult::Victory:
    {
      return setRandomScript({
        "./motionscripts/body-language-clapping.json",
        "./motionscripts/body-language-fist-pump.json",
        "./motionscripts/body-language-pelvic-thrust.json"
      });
    }
    case GameResult::Loss:
    {
      return setRandomScript({
        "./motionscripts/body-language-no.json",
        "./motionscripts/body-language-whoops.json"
      });
    }
    case GameResult::Undecided:
    default:
    {
      return {};
    }
  }
}

void GameOver::reset()
{
  d_result = GameResult::Undecided;
  d_playCount = 0;
  d_hasTerminated = false;
}

double GameOver::hasTerminated()
{
  return d_hasTerminated ? 1.0 : 0.0;
}
