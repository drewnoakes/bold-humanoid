#include "Agent/agent.hh"
#include "Camera/camera.hh"
#include "Config/config.hh"
#include "ImageCodec/PngCodec/pngcodec.hh"
#include "OptionTreeBuilder/AdHocOptionTreeBuilder/adhocoptiontreebuilder.hh"
#include "ThreadUtil/threadutil.hh"
#include "Version/version.hh"

#include <getopt.h>
#include <signal.h>
#include <syslog.h>

using namespace bold;
using namespace std;

unique_ptr<Agent> agent;

void printUsage()
{
  cout << endl;
  cout << "Options:" << endl;
  cout << endl;
  cout << ccolor::fore::lightblue << "  -c <file> " << ccolor::fore::white << "use specified configuration file (or --config)" << endl;
  cout << ccolor::fore::lightblue << "  -v        " << ccolor::fore::white << "verbose logging (or --verbose)" << endl;
  cout << ccolor::fore::lightblue << "  -vv       " << ccolor::fore::white << "trace logging" << endl;
  cout << ccolor::fore::lightblue << "  -h        " << ccolor::fore::white << "show these options (or --help)" << endl;
  cout << ccolor::fore::lightblue << "  -i <file> " << ccolor::fore::white << "use specified image file as camera feed (or --image)" << endl;
  cout << ccolor::fore::lightblue << "  --version " << ccolor::fore::white << "print git version details at time of build" << endl;
  cout << ccolor::reset;
}

void syslog(const char* msg)
{
  openlog("boldhumanoid", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
  syslog(LOG_NOTICE, "%s", msg);
  closelog();
}

void handleShutdownSignal(int sig)
{
  if (agent)
  {
    log::info("boldhumanoid") << "Received signal '" << strsignal(sig) << "' (" << sig << ") - stopping agent";
    syslog("Shutdown request received");
    agent->requestStop();
  }
}

vector<string> banners = {
  "\n╔══╗────╔╗╔╗╔╗──────╔╗╔═╗\n║╔╗╠═╦╗╔╝║║╚╝╠═╦═╗╔╦╣╚╣═╣\n║╔╗║╬║╚╣╬║║╔╗║╩╣╬╚╣╔╣╔╬═║\n╚══╩═╩═╩═╝╚╝╚╩═╩══╩╝╚═╩═╝",
  "\n╔══╗───╔╗──╔╗╔╗─╔╗───────╔╗\n║╔╗║───║║──║║║║─║║──────╔╝╚╗\n║╚╝╚╦══╣║╔═╝║║╚═╝╠══╦══╦╩╗╔╬══╗\n║╔═╗║╔╗║║║╔╗║║╔═╗║║═╣╔╗║╔╣║║══╣\n║╚═╝║╚╝║╚╣╚╝║║║─║║║═╣╔╗║║║╚╬══║\n╚═══╩══╩═╩══╝╚╝─╚╩══╩╝╚╩╝╚═╩══╝",
  "\n┏━━┓╋╋╋┏┓╋╋┏┓┏┓╋┏┓╋╋╋╋╋╋╋┏┓\n┃┏┓┃╋╋╋┃┃╋╋┃┃┃┃╋┃┃╋╋╋╋╋╋┏┛┗┓\n┃┗┛┗┳━━┫┃┏━┛┃┃┗━┛┣━━┳━━┳┻┓┏╋━━┓\n┃┏━┓┃┏┓┃┃┃┏┓┃┃┏━┓┃┃━┫┏┓┃┏┫┃┃━━┫\n┃┗━┛┃┗┛┃┗┫┗┛┃┃┃╋┃┃┃━┫┏┓┃┃┃┗╋━━┃\n┗━━━┻━━┻━┻━━┛┗┛╋┗┻━━┻┛┗┻┛┗━┻━━┛",
  "\n╭━━╮╱╱╱╭╮╱╱╭╮╭╮╱╭╮╱╱╱╱╱╱╱╭╮\n┃╭╮┃╱╱╱┃┃╱╱┃┃┃┃╱┃┃╱╱╱╱╱╱╭╯╰╮\n┃╰╯╰┳━━┫┃╭━╯┃┃╰━╯┣━━┳━━┳┻╮╭╋━━╮\n┃╭━╮┃╭╮┃┃┃╭╮┃┃╭━╮┃┃━┫╭╮┃╭┫┃┃━━┫\n┃╰━╯┃╰╯┃╰┫╰╯┃┃┃╱┃┃┃━┫╭╮┃┃┃╰╋━━┃\n╰━━━┻━━┻━┻━━╯╰╯╱╰┻━━┻╯╰┻╯╰━┻━━╯",
  " ____ ____ ____ ____ _________ ____ ____ ____ ____ ____ ____\n||B |||o |||l |||d |||       |||H |||e |||a |||r |||t |||s ||\n||__|||__|||__|||__|||_______|||__|||__|||__|||__|||__|||__||\n|/__\\|/__\\|/__\\|/__\\|/_______\\|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|",
  "\n/\\\\ /\\\\           /\\\\    /\\\\      /\\\\     /\\\\                         /\\\\\n/\\    /\\\\         /\\\\    /\\\\      /\\\\     /\\\\                         /\\\\\n/\\     /\\\\  /\\\\   /\\\\    /\\\\      /\\\\     /\\\\  /\\\\      /\\\\   /\\ /\\\\/\\/\\ //\\\\\\\\\n/\\\\\\ /\\   /\\\\  /\\\\/\\\\/\\\\ /\\\\      /\\\\\\\\\\\\ /\\\\/\\   /\\\\ /\\\\  /\\\\ /\\\\    /\\\\/\\\\\n/\\     /\\/\\\\    /\\/\\/\\   /\\\\      /\\\\     /\\/\\\\\\\\\\ /\\/\\\\   /\\\\ /\\\\    /\\\\  /\\\\\\\n/\\      /\\/\\\\  /\\\\/\\/\\   /\\\\      /\\\\     /\\/\\       /\\\\   /\\\\ /\\\\    /\\\\    /\\\\\n/\\\\\\\\ /\\\\   /\\\\  /\\\\\\/\\\\ /\\\\      /\\\\     /\\\\ /\\\\\\\\    /\\\\ /\\\\/\\\\\\     /\\/\\\\ /\\\\",
  "  _______       __    __     ___ ___                  __\n |   _   .-----|  .--|  |   |   Y   .-----.---.-.----|  |_.-----.\n |.  1   |  _  |  |  _  |   |.  1   |  -__|  _  |   _|   _|__ --|\n |.  _   |_____|__|_____|   |.  _   |_____|___._|__| |____|_____|\n |:  1    \\                 |:  |   |\n |::.. .  /                 |::.|:. |\n `-------'                  `--- ---'",
  " ______       __    __      _______                  __\n|   __ .-----|  .--|  |    |   |   .-----.---.-.----|  |_.-----.\n|   __ |  _  |  |  _  |    |       |  -__|  _  |   _|   _|__ --|\n|______|_____|__|_____|    |___|___|_____|___._|__| |____|_____|",
  "   _   _   _   _     _   _   _   _   _   _\n  / \\ / \\ / \\ / \\   / \\ / \\ / \\ / \\ / \\ / \\\n ( B ( o ( l ( d ) ( H ( e ( a ( r ( t ( s )\n  \\_/ \\_/ \\_/ \\_/   \\_/ \\_/ \\_/ \\_/ \\_/ \\_/",
  " _____     _   _    _____             _\n| __  |___| |_| |  |  |  |___ ___ ___| |_ ___\n| __ -| . | | . |  |     | -_| .'|  _|  _|_ -|\n|_____|___|_|___|  |__|__|___|__,|_| |_| |___|",
  "    ____        __    __   __  __                __\n   / __ )____  / ____/ /  / / / ___  ____ ______/ /______\n  / __  / __ \\/ / __  /  / /_/ / _ \\/ __ `/ ___/ __/ ___/\n / /_/ / /_/ / / /_/ /  / __  /  __/ /_/ / /  / /_(__  )\n/_____/\\____/_/\\__,_/  /_/ /_/\\___/\\__,_/_/   \\__/____/",
  " ____  __  __   ____    _  _ ____  __  ____ ____ ____\n(  _ \\/  \\(  ) (    \\  / )( (  __)/ _\\(  _ (_  _/ ___)\n ) _ (  O / (_/\\) D (  ) __ () _)/    \\)   / )( \\___ \\\n(____/\\__/\\____(____/  \\_)(_(____\\_/\\_(__\\_)(__)(____/",
  "                       )\n   (      (  (      ( /(                 )\n ( )\\     )\\ )\\ )   )\\())  (    ) (   ( /(\n )((_) ( ((_(()/(  ((_)\\  ))\\( /( )(  )\\()(\n((_)_  )\\ _  ((_))  _((_)/((_)(_)(()\\(_))/)\\\n | _ )((_| | _| |  | || (_))((_)_ ((_| |_((_)\n | _ / _ | / _` |  | __ / -_/ _` | '_|  _(_-<\n |___\\___|_\\__,_|  |_||_\\___\\__,_|_|  \\__/__/",
  "______       _     _   _   _                 _\n| ___ \\     | |   | | | | | |               | |\n| |_/ / ___ | | __| | | |_| | ___  __ _ _ __| |_ ___\n| ___ \\/ _ \\| |/ _` | |  _  |/ _ \\/ _` | '__| __/ __|\n| |_/ | (_) | | (_| | | | | |  __| (_| | |  | |_\\__ \\\n\\____/ \\___/|_|\\__,_| \\_| |_/\\___|\\__,_|_|   \\__|___/",
  "  ____        _     _   _   _                 _\n | __ )  ___ | | __| | | | | | ___  __ _ _ __| |_ ___\n |  _ \\ / _ \\| |/ _` | | |_| |/ _ \\/ _` | '__| __/ __|\n | |_) | (_) | | (_| | |  _  |  __| (_| | |  | |_\\__ \\\n |____/ \\___/|_|\\__,_| |_| |_|\\___|\\__,_|_|   \\__|___/"
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

void printVersion()
{
  cout << ccolor::fore::lightblue << "SHA1:        " << ccolor::reset << Version::GIT_SHA1 << endl
#if EIGEN_ALIGN
       << ccolor::fore::lightblue << "Eigen align: " << ccolor::reset << "Yes" << endl
#else
       << ccolor::fore::lightblue << "Eigen align: " << ccolor::reset << "No" << endl
#endif
       << ccolor::fore::lightblue << "Build type:  " << ccolor::reset << Version::BUILD_TYPE << endl
       << ccolor::fore::lightblue << "Build host:  " << ccolor::reset << Version::BUILT_ON_HOST_NAME << endl
#if INCLUDE_ASSERTIONS
       << ccolor::fore::lightblue << "Assertions:  " << ccolor::reset << "Yes" << endl
#else
       << ccolor::fore::lightblue << "Assertions:  " << ccolor::reset << "No" << endl
#endif
       << ccolor::fore::lightblue << "Commit date: " << ccolor::reset << Version::GIT_DATE << " (" << Version::describeTimeSinceGitDate() << ")" << endl
       << ccolor::fore::lightblue << "Message:     " << ccolor::reset << Version::GIT_COMMIT_SUBJECT << endl;
}

void logVersion()
{
  log::info("BUILD") << Version::GIT_SHA1 << " (committed " << Version::describeTimeSinceGitDate() << ")";
#if INCLUDE_ASSERTIONS
  log::info("ASSERTIONS") << "Yes";
#else
  log::info("ASSERTIONS") << "No";
#endif
#if EIGEN_ALIGN
  log::info("EIGEN_ALIGN") << "Yes";
#else
  log::info("EIGEN_ALIGN") << "No";
#endif
  log::info("BUILD_TYPE") << Version::BUILD_TYPE;
  log::info("BUILT_ON_HOST_NAME") << Version::BUILT_ON_HOST_NAME << "\n";
}

int main(int argc, char **argv)
{
  syslog("Starting boldhumanoid process");

  ThreadUtil::setThreadId(ThreadId::Main);

  srand(static_cast<unsigned int>(time(0)));

  // defaults
  string configurationFile = "configuration-agent.json";
  string imageFeedFile = "";
  log::minLevel = LogLevel::Info;

  int c;

  //
  // Process command line arguments
  //
  option longOptions[] = {
    {"config", required_argument, nullptr, 'c'},
    {"help", no_argument, nullptr, 'h'},
    {"image", required_argument, nullptr, 'i'},
    {"verbose", no_argument, nullptr, 'v'},
    {"version", no_argument, nullptr, 1},
    {0, 0, nullptr, 0}
  };

  int verboseCount = 0;
  int optionIndex;
  while ((c = getopt_long(argc, argv, "c:h:i:v", longOptions, &optionIndex)) != -1)
  {
    switch (c)
    {
      case 'c':
      {
        configurationFile = string{optarg};
        break;
      }
      case 'h':
      {
        printBanner();
        printUsage();
        exit(EXIT_SUCCESS);
      }
      case 'i':
      {
        imageFeedFile = string{optarg};
        break;
      }
      case 'v':
      {
        verboseCount++;
        break;
      }
      case 1:
      {
        printVersion();
        exit(EXIT_SUCCESS);
      }
      case '?':
      {
        // getopt_long already printed an error message
        printUsage();
        exit(EXIT_FAILURE);
      }
    }
  }

  if (verboseCount == 1)
    log::minLevel = LogLevel::Verbose;
  else if (verboseCount == 2)
    log::minLevel = LogLevel::Trace;

  auto startTime = Clock::getTimestamp();

  printBanner();

  logVersion();

  Config::initialise("configuration-metadata.json", configurationFile);

  agent.reset(new Agent());

  AdHocOptionTreeBuilder optionTreeBuilder;
  agent->setOptionTree(optionTreeBuilder.buildTree(agent.get()));

  Config::initialisationCompleted();

  if (imageFeedFile != "")
  {
    PngCodec pngCodec;
    cv::Mat image;
    if (!pngCodec.read(imageFeedFile, image))
      log::error("boldhumanoid") << "Unable to read image feed file: " << imageFeedFile;
    else
      agent->getCamera()->setImageFeed(image);
  }

  signal(SIGTERM, &handleShutdownSignal);
  signal(SIGINT, &handleShutdownSignal);

  log::info("boldhumanoid") << "Running Agent";

  agent->run();

  log::info("boldhumanoid") << "Finished after " << Clock::describeDurationSince(startTime);

  syslog("Exiting boldhumanoid process");

  return EXIT_SUCCESS;
}
