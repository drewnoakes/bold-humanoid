#include "visualcortex.hh"

#include "../ImageCodec/PngCodec/pngcodec.hh"
#include "../State/state.hh"
#include "../StateObject/AgentFrameState/agentframestate.hh"
#include "../StateObject/BodyState/bodystate.hh"
#include "../StateObject/CameraFrameState/cameraframestate.hh"

#include <rapidjson/prettywriter.h>

#include <fstream>
#include <sys/stat.h>

using namespace cv;
using namespace bold;
using namespace rapidjson;
using namespace std;

void VisualCortex::saveImage(cv::Mat const& image, std::map<uchar,Colour::bgr>* palette)
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

  // Encode the image
  PngCodec codec;
  vector<uchar> imageBuffer;
  codec.encode(image, imageBuffer, palette);

  // Write it to a file
  ofstream imageFile;
  imageFile.open(fileName.str());
  imageFile.write(reinterpret_cast<char*>(imageBuffer.data()), imageBuffer.size());
  imageFile.close();

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
      writer.String("hostname");
      writer.String(hostName);
    }

    // Current date and time
    writer.String("date");
    writer.String(dateTimeString);

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
    StateObject::writeJsonOrNull(writer, State::get<BodyState>(StateTime::CameraImage));

    // Camera Frame
    writer.String("cameraFrame");
    StateObject::writeJsonOrNull(writer, State::get<CameraFrameState>());

    // Agent Frame
    writer.String("agentFrame");
    StateObject::writeJsonOrNull(writer, State::get<AgentFrameState>());
  }
  writer.EndObject();

  // Write the JSON file
  ofstream jsonFile;
  jsonFile.open(fileName.str());
  jsonFile << buffer.GetString();
  jsonFile.close();
}
