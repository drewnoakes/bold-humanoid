#include "visualcortex.ih"

#include "../AgentState/agentstate.hh"
#include "../StateObject/AgentFrameState/agentframestate.hh"
#include "../StateObject/BodyState/bodystate.hh"

#include <rapidjson/prettywriter.h>

#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

using namespace rapidjson;
using namespace cv;

void VisualCortex::saveImage(cv::Mat const& image)
{
  time_t rawtime;
  time(&rawtime);

  tm* timeinfo = localtime(&rawtime);

  char dateTimeString[80];
  // %G yyyy
  // %m MM
  // %d dd
  // %H HH
  // %M mm
  // %S ss
  //
  strftime(dateTimeString, 80, "%G%m%d-%H:%M:%S", timeinfo);

  string folderName = "captures";
  mkdir(folderName.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);

  stringstream fileName;
  fileName << folderName << "/" << dateTimeString << ".png";
  log::info("VisualCortex::saveImage") << "Saving " << fileName.str();

  // Write the image file
  cv::imwrite(fileName.str(), image);

  // Clear the filename
  fileName.str(std::string());

  fileName << folderName << "/" << dateTimeString << ".json";
  log::info("VisualCortex::saveImage") << "Saving " << fileName.str();

  // Build up the output JSON
  StringBuffer buffer;
  PrettyWriter<StringBuffer> writer(buffer);
  writer.StartObject();
  {
    // Host name
    char hostName[80];
    if (gethostname(hostName, 80) == -1)
    {
      log::warning("VisualCortex::saveImage") << "gethostname failed: " << strerror(errno);
    }
    else
    {
      writer.String("hostname").String(hostName);
    }

    // Current date and time
    writer.String("date").String(dateTimeString);

    // Camera settings
    writer.String("camera-settings");
    writer.StartObject();
    {
      for (SettingBase* setting : Config::getSettings("camera.settings"))
      {
        writer.String(setting->getName().c_str());
        setting->writeJsonValue(writer);
      }
    }
    writer.EndObject();

    // Body pose
    writer.String("body");
    StateObject::writeJsonOrNull(writer, AgentState::get<BodyState>(StateTime::CameraImage));

    // Camera Frame
    writer.String("cameraFrame");
    StateObject::writeJsonOrNull(writer, AgentState::get<CameraFrameState>());

    // Agent Frame
    writer.String("agentFrame");
    StateObject::writeJsonOrNull(writer, AgentState::get<AgentFrameState>());
  }
  writer.EndObject();

  // Write the JSON file
  ofstream file;
  file.open(fileName.str());
  file << buffer.GetString();
  file.close();
}
