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

  if (!d.HasString("family"))
  {
    cerr << "[DataStreamer::processCommand] No family specified in received command JSON" << endl;
    return;
  }

  string family = d["family"].GetString();

  unsigned id;
  if (!d.TryGetUintValue("id", &id))
  {
    cerr << "[DataStreamer::processCommand] No 'id' specified in received command JSON" << endl;
    return;
  }

  // find the command
  map<unsigned, shared_ptr<Control const>>& controlsById = d_controlsByIdByFamily[family];

  auto it = controlsById.find(id);

  if (it == controlsById.end())
  {
    cerr << "[DataStreamer::processCommand] Ignoring unknown command with family '" << family << "' and id " << id << endl;
    return;
  }

  shared_ptr<Control const> controls = it->second;

  if (!controls->handleRequest(d))
  {
    cerr << "[DataStreamer::processCommand] Processing failed for: " << json << endl;
  }
}
