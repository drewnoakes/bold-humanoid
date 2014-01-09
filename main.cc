#include "Agent/agent.hh"
#include "Config/config.hh"
//#include "MotionScript/motionscript.hh"
#include "OptionTree/optiontree.hh"
#include "OptionTreeBuilder/AdHocOptionTreeBuilder/adhocoptiontreebuilder.hh"
// #include "RobotisMotionFile/robotismotionfile.hh"
//#include "ThreadId/threadid.hh"
#include "util/ccolor.hh"
#include "util/log.hh"
#include "version.hh"

#include <limits>
#include <vector>
#include <string>
#include <signal.h>

using namespace bold;
using namespace std;

unique_ptr<Agent> agent;

void printUsage()
{
  cout << endl;
  cout << "Options:" << endl;
  cout << endl;
  cout << ccolor::fore::lightblue << "  -u <num>  " << ccolor::fore::white << "uniform number (or --unum)" << endl;
  cout << ccolor::fore::lightblue << "  -t <num>  " << ccolor::fore::white << "team number (or --team)" << endl;
  cout << ccolor::fore::lightblue << "  -c <file> " << ccolor::fore::white << "use specified configuration file (or --config)" << endl;
  cout << ccolor::fore::lightblue << "  -v        " << ccolor::fore::white << "verbose logging (or --verbose)" << endl;
  cout << ccolor::fore::lightblue << "  -q        " << ccolor::fore::white << "quiet/don't speak' (or --quiet)" << endl;
  cout << ccolor::fore::lightblue << "  -h        " << ccolor::fore::white << "show these options (or --help)" << endl;
  cout << ccolor::fore::lightblue << "  -version  " << ccolor::fore::white << "print git version details at time of build" << endl;
  cout << ccolor::reset;
}

void handleShutdownSignal(int sig)
{
  if (agent)
  {
    log::info("boldhumanoid") << "Stopping Agent";
    agent->requestStop();
  }
}

// void convertMotionFile()
// {
//   vector<shared_ptr<MotionScript const>> motionScripts;
//
//   auto motionScriptFileName = "./motion_4096.bin";
//   log::info("convertMotionFile") << "Processing Robotis-formatted motion file: " << motionScriptFileName;
//   auto motionScriptFile = RobotisMotionFile(motionScriptFileName);
//   auto rootPageIndices = motionScriptFile.getSequenceRootPageIndices();
//   for (uchar rootPageIndex : rootPageIndices)
//   {
//     stringstream ss;
//     ss << "./motionscripts/motion_4096." << (int)rootPageIndex << ".json";
//     shared_ptr<MotionScript> motionScript = motionScriptFile.toMotionScript(rootPageIndex);
//     motionScripts.push_back(motionScript);
//     motionScript->writeJsonFile(ss.str());
//   }
//   motionScriptFile.toDotText(cout);
// }

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
  cout << ccolor::bold << ccolor::fore::lightmagenta
       << banners[rand() % banners.size()]
       << endl << endl << ccolor::reset;
}

int main(int argc, char **argv)
{
  srand(time(0));

//  convertMotionFile();

  // defaults
  unsigned teamNumber = -1;
  unsigned uniformNumber = 0;
  bool useSpeech = true;

  string configurationFile("configuration.json");
  log::minLevel = LogLevel::Info;

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

  //
  // Process command line arguments
  //
  for (int i = 1; i < argc; ++i)
  {
    string arg(argv[i]);
    if (arg == "-h" || arg == "--help")
    {
      printBanner();
      printUsage();
      return 0;
    }
    else if (arg == "-t" || arg == "--team")
    {
      teamNumber = atoi(nextArg(&i));
    }
    else if (arg == "-u" || arg == "--unum")
    {
      uniformNumber = atoi(nextArg(&i));
    }
    else if (arg == "-c" || arg == "--config")
    {
      configurationFile = nextArg(&i);
    }
    else if (arg == "-v" || arg == "--verbose")
    {
      log::minLevel = LogLevel::Verbose;
    }
    else if (arg == "-q" || arg == "--quiet")
    {
      useSpeech = false;
    }
    else if (arg == "--version")
    {
      cout << Version::GIT_SHA1 << endl
           << Version::GIT_DATE << endl
           << Version::GIT_COMMIT_SUBJECT << endl;
      return 0;
    }
    else
    {
      log::error() << "Unknown argument: " << arg;
      printUsage();
      return -1;
    }
  }

  if (uniformNumber == 0)
  {
    log::error() << "You must provide a uniform number (-u)";
    printUsage();
    return -1;
  }

  printBanner();

  log::info() << Version::GIT_SHA1 << ")\n";

  Config::initialise("configuration-metadata.json", configurationFile);

  if (teamNumber == -1)
    teamNumber = Config::getStaticValue<int>("team.number");

  log::info("boldhumanoid") << "Team number " << teamNumber << ", uniform number " << uniformNumber;

  agent.reset(new Agent(useSpeech));

  Config::initialisationCompleted();

  agent->setTeamNumber(teamNumber);
  agent->setUniformNumber(uniformNumber);

  AdHocOptionTreeBuilder optionTreeBuilder;
  auto optionTree = optionTreeBuilder.buildTree(teamNumber,
                                                uniformNumber,
                                                agent.get(),
                                                agent->getDataStreamer(),
                                                agent->getDebugger(),
                                                agent->getCameraModel(),
                                                agent->getAmbulator(),
                                                agent->getMotionScriptModule(),
                                                agent->getHeadModule(),
                                                agent->getWalkModule(),
                                                agent->getFallDetector());

  agent->setOptionTree(move(optionTree));

  signal(SIGTERM, &handleShutdownSignal);
  signal(SIGINT, &handleShutdownSignal);

  log::info("boldhumanoid") << "Running Agent";
  agent->run();

  log::info("boldhumanoid") << "Finished";

  return 0;
}
