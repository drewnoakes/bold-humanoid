#include <iostream>

#include <libwebsockets.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "Clock/clock.hh"
#include "Config/config.hh"
#include "StateObject/TeamState/teamstate.hh"
#include "UDPSocket/udpsocket.hh"
#include "Version/version.hh"
#include "JointId/jointid.hh"

using namespace bold;
using namespace std;
using namespace rapidjson;

struct Session
{
  std::shared_ptr<std::vector<uchar> const> data;
  /** The number of bytes sent from the front message in the queue. */
  unsigned bytesSent;
};

vector<Session*> sessions;

int websocketCallback(libwebsocket_context* context, libwebsocket* wsi, libwebsocket_callback_reasons reason, void* user, void* in, size_t len)
{
  Session* session = reinterpret_cast<Session*>(user);

  switch (reason)
  {
    case LWS_CALLBACK_ESTABLISHED:
    {
      sessions.push_back(session);
      log::info("WebSockets") << "Client connected (" << sessions.size() << " active)";
      return 0;
    }
    case LWS_CALLBACK_CLOSED:
    {
      auto it = find(sessions.begin(), sessions.end(), session);
      if (it != sessions.end())
        sessions.erase(it);
      log::info("WebSockets") << "Client disconnected (" << sessions.size() << " active)";
      return 0;
    }
    case LWS_CALLBACK_SERVER_WRITEABLE:
    {
      // Fill the outbound pipe with frames of data
      while (!lws_send_pipe_choked(wsi) && session->data)
      {
        shared_ptr<vector<uchar> const> const& str = session->data;
        ASSERT(str);
        uint totalSize = str.get()->size();

        ASSERT(session->bytesSent < totalSize);

        const uchar* start = str.get()->data() + session->bytesSent;

        uint remainingSize = totalSize - session->bytesSent;
        uint frameSize = min(2048u, remainingSize);
        uchar frameBuffer[LWS_SEND_BUFFER_PRE_PADDING + frameSize + LWS_SEND_BUFFER_POST_PADDING];
        uchar *p = &frameBuffer[LWS_SEND_BUFFER_PRE_PADDING];

        memcpy(p, start, frameSize);

        int writeMode = session->bytesSent == 0
          ? LWS_WRITE_TEXT
          : LWS_WRITE_CONTINUATION;

        if (frameSize != remainingSize)
          writeMode |= LWS_WRITE_NO_FIN;

        int res = libwebsocket_write(wsi, p, frameSize, (libwebsocket_write_protocol)writeMode);

        if (res < 0)
        {
          lwsl_err("ERROR %d writing to socket (control)\n", res);
          return 1;
        }

        session->bytesSent += frameSize;

        if (session->bytesSent == totalSize)
        {
          // Done sending this queue item, so ditch it, reset and loop around again
          session->data = nullptr;
          session->bytesSent = 0;
          break;
        }
      }

      // Queue for more writing later on if we still have data remaining
      if (session->data)
        libwebsocket_callback_on_writable(context, wsi);

      return 0;
    }
    default:
    {
      return 0;
    }
  }
}

void printUsage()
{
  cout << endl;
  cout << "Options:" << endl;
  cout << endl;
  cout << ccolor::fore::lightblue << "  -c <file> " << ccolor::fore::white << "use specified configuration file (or --config)" << endl;
  cout << ccolor::fore::lightblue << "  -v        " << ccolor::fore::white << "verbose logging (or --verbose)" << endl;
  cout << ccolor::fore::lightblue << "  -r        " << ccolor::fore::white << "produce random data for testing (or --randomize)" << endl;
  cout << ccolor::fore::lightblue << "  -h        " << ccolor::fore::white << "show these options (or --help)" << endl;
  cout << ccolor::fore::lightblue << "  --version " << ccolor::fore::white << "print git version details at time of build" << endl;
  cout << ccolor::reset;
}

vector<string> banners = {
  " _____     ______     ______     __     __     ______     ______     __     _____     ______     ______   \n/\\  __-.  /\\  == \\   /\\  __ \\   /\\ \\  _ \\ \\   /\\  == \\   /\\  == \\   /\\ \\   /\\  __-.  /\\  ___\\   /\\  ___\\  \n\\ \\ \\/\\ \\ \\ \\  __<   \\ \\  __ \\  \\ \\ \\/ \".\\ \\  \\ \\  __<   \\ \\  __<   \\ \\ \\  \\ \\ \\/\\ \\ \\ \\ \\__ \\  \\ \\  __\\  \n \\ \\____-  \\ \\_\\ \\_\\  \\ \\_\\ \\_\\  \\ \\__/\".~\\_\\  \\ \\_____\\  \\ \\_\\ \\_\\  \\ \\_\\  \\ \\____-  \\ \\_____\\  \\ \\_____\\\n  \\/____/   \\/_/ /_/   \\/_/\\/_/   \\/_/   \\/_/   \\/_____/   \\/_/ /_/   \\/_/   \\/____/   \\/_____/   \\/_____/",
  "\n888~-_   888~-_        e      Y88b         / 888~~\\  888~-_   888 888~-_    e88~~\\  888~~ \n888   \\  888   \\      d8b      Y88b       /  888   | 888   \\  888 888   \\  d888     888___\n888    | 888    |    /Y88b      Y88b  e  /   888 _/  888    | 888 888    | 8888 __  888   \n888    | 888   /    /  Y88b      Y88bd8b/    888  \\  888   /  888 888    | 8888   | 888   \n888   /  888_-~    /____Y88b      Y88Y8Y     888   | 888_-~   888 888   /  Y888   | 888   \n888_-~   888 ~-_  /      Y88b      Y  Y      888__/  888 ~-_  888 888_-~    \"88__/  888___",
  "\n,-.  ,-.   ,.  ,   . ,-.  ,-.  , ,-.   ,-. ,--.\n|  \\ |  ) /  \\ | . | |  ) |  ) | |  \\ /    |   \n|  | |-<  |--| | ) ) |-<  |-<  | |  | | -. |-  \n|  / |  \\ |  | |/|/  |  ) |  \\ | |  / \\  | |   \n`-'  '  ' '  ' ' '   `-'  '  ' ' `-'   `-' `--'",
  "\n.%%%%%...%%%%%....%%%%...%%...%%..%%%%%...%%%%%...%%%%%%..%%%%%....%%%%...%%%%%%.\n.%%..%%..%%..%%..%%..%%..%%...%%..%%..%%..%%..%%....%%....%%..%%..%%......%%.....\n.%%..%%..%%%%%...%%%%%%..%%.%.%%..%%%%%...%%%%%.....%%....%%..%%..%%.%%%..%%%%...\n.%%..%%..%%..%%..%%..%%..%%%%%%%..%%..%%..%%..%%....%%....%%..%%..%%..%%..%%.....\n.%%%%%...%%..%%..%%..%%...%%.%%...%%%%%...%%..%%..%%%%%%..%%%%%....%%%%...%%%%%%.\n.................................................................................",
  " _____   ______         _  _  _ ______ ______  _____ _____    ______ _______ \n(____ \\ (_____ \\   /\\  | || || (____  (_____ \\(_____|____ \\  / _____|_______)\n _   \\ \\ _____) ) /  \\ | || || |____)  )____) )  _   _   \\ \\| /  ___ _____   \n| |   | (_____ ( / /\\ \\| ||_|| |  __  (_____ (  | | | |   | | | (___)  ___)  \n| |__/ /      | | |__| | |___| | |__)  )    | |_| |_| |__/ /| \\____/| |_____ \n|_____/       |_|______|\\______|______/     |_(_____)_____/  \\_____/|_______)",
  "______   ______ _______ _  _  _ ______   ______ _____ ______   ______ _______\n |     \\ |_____/ |_____| |  |  | |_____] |_____/   |   |     \\ |  ____ |______\n |_____/ |    \\_ |     | |__|__| |_____] |    \\_ __|__ |_____/ |_____| |______",
  "   __   ___    _   _   __ ___   ___   __ __    __   ___\n  /  \\ / o | .' \\ ///7/ // o.) / o | / //  \\ ,'_/  / _/\n / o |/  ,' / o /| V V // o \\ /  ,' / // o |/ /_n / _/ \n/__,'/_/`_\\/_n_/ |_n_,'/___,'/_/`_\\/_//__,' |__,'/___/ ",
  " __  __          __  __   __  __  __\n|  \\|__) /\\ |  ||__)|__)||  \\/ _ |_ \n|__/| \\ /--\\|/\\||__)| \\ ||__/\\__)|__"
};

void printBanner()
{
  if (log::isStdOutRedirected())
  {
    cout << banners[rand() % banners.size()] << endl << endl;
  }
  else
  {
    cout << ccolor::bold << ccolor::fore::lightmagenta
      << banners[rand() % banners.size()]
      << endl << endl << ccolor::reset;
  }
}

libwebsocket_protocols* d_protocols = new libwebsocket_protocols[2];

void queueBytes(char const* data, size_t len)
{
  //
  // Process WebSocket clients
  //
  auto str = make_shared<vector<uchar>>(len);
  memcpy(str->data(), data, len);
  for (auto const& session : sessions)
  {
    session->data = str;
    session->bytesSent = 0;
  }
  libwebsocket_callback_on_writable_all_protocol(&d_protocols[0]);
}

void queueRandomMessage()
{
  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);

  static int teamNumber = 1 + (std::rand() % 10);
  static int teamColour = 1 + (std::rand() % 2);
  static vector<string> names = { "nimue", "gareth", "oberon", "bors", "dagonet", "ywain", "tor" };
  static vector<PlayerActivity> playerActivities = { PlayerActivity::ApproachingBall, PlayerActivity::AttackingGoal, PlayerActivity::Other, PlayerActivity::Positioning, PlayerActivity::Waiting };
  static vector<PlayerStatus> playerStatuses = { PlayerStatus::Active, PlayerStatus::Inactive, PlayerStatus::Paused, PlayerStatus::Penalised };
  static auto startTime = Clock::getTimestamp();

  srand(time(nullptr));

  const int MaxPlayerNum = 6;

  int unum = (std::rand() % MaxPlayerNum) + 1;
  string playerName = names[unum - 1];

  writer.StartObject();
  {
    writer.String("unum").Int(1 + (std::rand() % MaxPlayerNum));
    writer.String("team").Int(teamNumber);
    writer.String("col").Int(teamColour);
    stringstream host;
    host << "darwin" << unum;
    writer.String("host").String(host.str().c_str());
    writer.String("name").String(playerName.c_str());
    writer.String("ver").String(Version::GIT_SHA1.c_str());
    writer.String("built").String(Version::BUILT_ON_HOST_NAME.c_str());
    writer.String("uptime").Uint(static_cast<uint>(Clock::getSecondsSince(startTime)));

    writer.String("role").String(getPlayerRoleString(unum == 1 ? PlayerRole::Keeper : PlayerRole::Striker).c_str());

    srand(unum);

    writer.String("activity").String(getPlayerActivityString(playerActivities[rand() % playerActivities.size()]).c_str());
    writer.String("status").String(getPlayerStatusString(playerStatuses[rand() % playerStatuses.size()]).c_str());

    writer.String("fpsThink").Double(25 + ((rand() % 100) / 10.0));
    writer.String("fpsMotion").Double(100 + ((rand() % 400) / 10.0));

    writer.String("agent");
    writer.StartObject();
    {
      if (rand() % 10 > 5)
      {
        writer.String("ball")
          .StartArray()
          .Double((rand() % 1000) * FieldMap::getFieldLengthX())
          .Double((rand() % 1000) * FieldMap::getFieldLengthY())
          .EndArray();
      }

      if (rand() % 10 > 5)
      {
        writer.String("goals")
          .StartArray()
            .StartArray()
            .Double((rand() % 1000) * FieldMap::getFieldLengthX())
            .Double((rand() % 1000) * FieldMap::getFieldLengthY())
            .EndArray();
        if (rand() % 10 > 5)
        {
          writer.StartArray()
            .Double((rand() % 1000) * FieldMap::getFieldLengthX())
            .Double((rand() % 1000) * FieldMap::getFieldLengthY())
            .EndArray();
        }
        writer.EndArray();
      }
    }
    writer.EndObject();

    writer.String("game");
    writer.StartObject();
    {
      static vector<string> playModes = {"Initial", "Ready", "Set", "Playing", "Finished"};
      writer.String("mode").String(playModes[std::rand()%playModes.size()].c_str());
      writer.String("age").Uint(std::rand() % 1000u);
    }
    writer.EndObject();

    writer.String("hw");
    writer.StartObject();
    {
      writer.String("volt").Double(10.7 + ((rand() % 35) / 10.0));
      writer.String("power").Bool(rand() % 1);
      writer.String("temps").StartArray();
      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
        writer.Uint(30 + (rand() % 30));
      writer.EndArray();
    }
    writer.EndObject();

    writer.String("teammates");
    writer.StartArray();
    {
      writer.StartObject();
      {
        writer.String("unum").Int(1 + (rand() % MaxPlayerNum));
        writer.String("ms").Int(std::rand() % 1000u);
      }
      writer.EndObject();
    }
    writer.EndArray();

    vector<string> ranOptions = {"win", "stop-walking", "look-at-feet", "build-stationary-map", "motion-script"};
    vector<pair<string,string>> fsmStates = { {"win", "playing"}, {"win", "getUp"} };

    writer.String("options");
    writer.Array(
      ranOptions.begin(), ranOptions.end(),
      [&](string const& option) { writer.String(option.c_str()); });

    writer.String("fsms");
    writer.Array(
      fsmStates.begin(), fsmStates.end(),
      [&](pair<string,string> fsmState) { writer.StartObject()
        .String("fsm").String(fsmState.first.c_str())
        .String("state").String(fsmState.second.c_str())
        .EndObject();
      });
  }
  writer.EndObject();

  queueBytes(buffer.GetString(), buffer.GetSize());
}

int main(int argc, char **argv)
{
  srand(time(0));

  lws_set_log_level(LLL_ERR | LLL_WARN, nullptr);

  // TODO: use getopt
  auto nextArg = [&](int* i) -> char*
  {
    if (*i == argc - 1)
    {
      // No more arguments. Error!
      log::error() << "Insufficient arguments";
      exit(-1);
    }
    int j = *i + 1;
    *i = j;
    return argv[j];
  };

  string configurationFile = "configuration-agent.json";
  log::minLevel = LogLevel::Info;
  bool randomise = false;

  for (int i = 1; i < argc; ++i)
  {
    string arg(argv[i]);
    if (arg == "-h" || arg == "--help")
    {
      printBanner();
      printUsage();
      return 0;
    }
    else if (arg == "-c" || arg == "--config")
    {
      configurationFile = nextArg(&i);
    }
    else if (arg == "-v" || arg == "--verbose")
    {
      log::minLevel = LogLevel::Verbose;
    }
    else if (arg == "-r" || arg == "--randomize")
    {
      randomise = true;
    }
    else if (arg == "--version")
    {
      cout << ccolor::fore::lightblue << "SHA1:        " << ccolor::reset << Version::GIT_SHA1 << endl
#if EIGEN_ALIGN
           << ccolor::fore::lightblue << "Eigen align: " << ccolor::reset << "Yes" << endl
#else
        << ccolor::fore::lightblue << "Eigen align: " << ccolor::reset << "No" << endl
#endif
        << ccolor::fore::lightblue << "Build type:  " << ccolor::reset << Version::BUILD_TYPE << endl
#if INCLUDE_ASSERTIONS
        << ccolor::fore::lightblue << "Assertions:  " << ccolor::reset << "Yes" << endl
#else
           << ccolor::fore::lightblue << "Assertions:  " << ccolor::reset << "No" << endl
#endif
        << ccolor::fore::lightblue << "Commit date: " << ccolor::reset << Version::GIT_DATE << " (" << Version::describeTimeSinceGitDate() << ")" << endl
        << ccolor::fore::lightblue << "Message:     " << ccolor::reset << Version::GIT_COMMIT_SUBJECT << endl;
      return 0;
    }
    else
    {
      log::error() << "Unknown argument: " << arg;
      printUsage();
      return -1;
    }
  }

  printBanner();

  Config::initialise("configuration-metadata.json", configurationFile);

  // TODO support --random option for front-end testing and demonstration purposes
  // TODO only log output when number of robots changes, or a client connects/disconnects

  int udpPort = Config::getStaticValue<int>("drawbridge.udp-port");
  int wsPort = Config::getStaticValue<int>("drawbridge.websocket-port");

  //
  // UDP socket for listening
  //

  UDPSocket socket;
  socket.setBlocking(false);
  socket.bind("", udpPort);

  //
  // WebSocket for publishing
  //

                   // name, callback, per-session-data-size, rx-buffer-size, no-buffer-all-partial-tx
  d_protocols[0] = { "drawbridge", websocketCallback, sizeof(Session), 0, 0 };
  // Mark the end of the protocols
  d_protocols[1] = { nullptr, nullptr, 0, 0, 0 };

  lws_context_creation_info contextInfo;
  memset(&contextInfo, 0, sizeof(contextInfo));
  contextInfo.port = wsPort;
  contextInfo.protocols = d_protocols;
  contextInfo.gid = contextInfo.uid = -1;
//  contextInfo.user = this;
  libwebsocket_context* d_context = libwebsocket_create_context(&contextInfo);

  if (d_context == nullptr)
  {
    log::error("WebSockets") << "libwebsocket context creation failed";
    exit(1);
  }
  log::info("WebSockets") << "Listening on TCP port " << wsPort;

  //
  // Start tireless loop
  //

  while (true)
  {
    static constexpr uint MaxMessageSize = 1024*1024;

    //
    // Listen for UDP packet
    //

    static char data[MaxMessageSize];

    int bytesRead;
    while ((bytesRead = socket.receive(data, MaxMessageSize)) != 0)
    {
      // Returns zero bytes when no message available (non-blocking)

      // Returns -1 when an error occurred. UDPSocket logs the error.
      if (bytesRead < 0)
        break;

      static bool seenYet = false;
      if (!likely(seenYet))
      {
        log::info("drawbridge") << "Received first message";
        seenYet = true;
      }

      queueBytes(data, bytesRead);
    }

    if (randomise)
    {
      // Produce one random player's message
      queueRandomMessage();

      usleep(200 * 1000); // 0.2 sec
    }

    //
    // Process whatever else needs doing on the socket (new clients, etc)
    // This is normally very fast
    //
    libwebsocket_service(d_context, 0);

    //
    // Sleep a while to avoid pegging the CPU at 100%
    //
    usleep(50000); // 50ms
  }
}
