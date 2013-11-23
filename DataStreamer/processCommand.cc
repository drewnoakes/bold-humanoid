#include "datastreamer.ih"

void DataStreamer::processCommand(std::string json)
{
  cout << "[DataStreamer::processCommand] Processing: " << json << endl;

  rapidjson::Document d;

  d.Parse<0>(json.c_str());

  if (d.HasParseError())
  {
    cerr << ccolor::error << "[DataStreamer::processCommand] Error parsing command JSON" << ccolor::reset << endl;
    return;
  }

  // TODO SETTINGS determine whether this is an action, or a setting change

  char const* id;
  if (!d.TryGetStringValue("id", &id))
  {
    cerr << ccolor::error << "[DataStreamer::processCommand] No 'id' specified in received command JSON" << ccolor::reset << endl;
    return;
  }

  Config::getAction(string(id));
}
