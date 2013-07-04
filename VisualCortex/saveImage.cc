#include "visualcortex.ih"

#include "../AgentState/agentstate.hh"
#include "../StateObject/BodyState/bodystate.hh"

#include <rapidjson/prettywriter.h>

#include <iostream>
#include <fstream>
#include <stdio.h>      /* puts */
#include <time.h>       /* time_t, struct tm, time, localtime, strftime */
#include <sys/stat.h>
#include <sys/types.h>

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
  cout << "[VisualCortex::saveImage] Saving " << fileName.str() << endl;

  // Write the image file
  cv::imwrite(fileName.str(), image);

  // Clear the filename
  fileName.str(std::string());

  fileName << folderName << "/" << dateTimeString << ".json";
  cout << "[VisualCortex::saveImage] Saving " << fileName.str() << endl;

  // Build up the output JSON
  StringBuffer buffer;
  PrettyWriter<StringBuffer> writer(buffer);
  writer.StartObject();
  {
    // Current date and time
    writer.String("date").String(dateTimeString);

    // Camera settings
    writer.String("camera");
    writer.StartObject();
    {
      for (shared_ptr<Control const> const& control : d_camera->getControls())
        writer.String(control->getName().c_str()).Int(control->getValue());
    }
    writer.EndObject();

    // Body pose
    shared_ptr<BodyState const> bodyState = AgentState::get<BodyState>();
    bodyState->writeJson(writer);
  }
  writer.EndObject();

  // Write the JSON file
  ofstream file;
  file.open(fileName.str());
  file << buffer.GetString();
  file.close();
}
