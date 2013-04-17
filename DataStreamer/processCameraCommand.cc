#include "datastreamer.ih"

void DataStreamer::processCameraCommand(std::string json)
{
  cout << "[DataStreamer::processCameraCommand] Processing: " << json << endl;

  rapidjson::Document d;

  d.Parse<0>(json.c_str());

  if (d.HasParseError())
  {
    cerr << "[DataStreamer::processCameraCommand] Error parsing command JSON" << endl;
    return;
  }

  if (!d.HasString("family"))
  {
    cerr << "[DataStreamer::processCameraCommand] No family specified in received command JSON" << endl;
    return;
  }

  string family = d["family"].GetString();

  unsigned id;
  if (!d.TryGetUintValue("id", &id))
  {
    cerr << "[DataStreamer::processCameraCommand] No 'id' specified in received command JSON" << endl;
    return;
  }

  // find the command
  map<unsigned, Control>& controlsById = d_controlsByIdByFamily[family];

  auto it = controlsById.find(id);

  if (it == controlsById.end())
  {
    cerr << "[DataStreamer::processCameraCommand] Ignoring unknown command with family '" << family << "' and id " << id << endl;
    return;
  }

  Control& controls = it->second;

  if (!controls.handleRequest(d))
  {
    cerr << "[DataStreamer::processCameraCommand] Processing failed for: " << json << endl;
  }
}