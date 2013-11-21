#include "datastreamer.ih"

void DataStreamer::processCommand(std::string json)
{
  cout << "[DataStreamer::processCommand] Processing: " << json << endl;

  rapidjson::Document d;

  d.Parse<0>(json.c_str());

  if (d.HasParseError())
  {
    cerr << "[DataStreamer::processCommand] Error parsing command JSON" << endl;
    return;
  }

  // TODO SETTINGS determine whether this is an action, or a setting change

  char const* id;
  if (!d.TryGetStringValue("id", &id))
  {
    cerr << "[DataStreamer::processCommand] No 'id' specified in received command JSON" << endl;
    return;
  }

  Config::getAction(string(id));
}
