#include "datastreamer.ih"

// TODO send initial state to browser upon connect (image type, period, layers)
// TODO support command for: drawing detected blobs
// TODO support command for: drawing detected lines
// TODO support command for: drawing projected field lines
// TODO add/remove pass handlers depending upon the selected image type

void DataStreamer::processCameraCommand(std::string json)
{
  // Parse JSON
  rapidjson::Document d;

  d.Parse<0>(json.c_str());

  if (d.HasParseError())
  {
    cerr << "[DataStreamer::processCameraCommand] Error parsing JSON request" << endl;
    return;
  }

  if (!d.HasString("command"))
  {
    cerr << "[DataStreamer::processCameraCommand] No command specified in received message" << endl;
    return;
  }

  string command(d["command"].GetString());

  if (command == "setControl")
  {
    // { "command": "setControl", "id": 1, "val": 123 }

    unsigned controlId;
    unsigned controlVal;
    if (!d.TryGetUintMember("id", &controlId) || !d.TryGetUintMember("val", &controlVal))
    {
      cerr << "[DataStreamer::processCameraCommand] Invalid setControl command" << endl;
      return;
    }

    auto control = d_camera->getControl(controlId);
    if (control.hasValue())
      control.value()->setValue(controlVal);
  }
  else if (command == "selectStream")
  {
    // { "command": "selectStream", "id": 1 }

    unsigned imageTypeId;
    if (!d.TryGetUintMember("id", &imageTypeId))
    {
      cerr << "[DataStreamer::processCameraCommand] Invalid selectStream command" << endl;
      return;
    }

    d_imageType = (ImageType)imageTypeId;
  }
  else if (command == "controlHead")
  {
    // { "command": "controlHead", "action": "<" }

    if (!d.HasString("action"))
    {
      cerr << "[DataStreamer::processCameraCommand] Invalid controlHead command" << endl;
      return;
    }
    string action(d["action"].GetString());

    Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);

    if (action == "<")
      Head::GetInstance()->MoveByAngleOffset(5,0);
    else if (action == ">")
      Head::GetInstance()->MoveByAngleOffset(-5,0);
    else if (action == "^")
      Head::GetInstance()->MoveByAngleOffset(0,5);
    else if (action == "v")
      Head::GetInstance()->MoveByAngleOffset(0,-5);
  }
  else if (command == "framePeriod")
  {
    // { "command": "framePeriod", "period": "5" }

    unsigned period;
    if (!d.TryGetUintMember("period", &period) || period == 0)
    {
      cerr << "[DataStreamer::processCameraCommand] Invalid framePeriod command" << endl;
      return;
    }

    d_streamFramePeriod = period;
  }
  else if (command == "setLayerVisibility")
  {
    // { "command": "setLayerVisibility", "layer": "blobs", "visible": true }

    if (!d.HasMember("layer") || !d["layer"].IsString() || !d.HasMember("visible") || !d["visible"].IsBool())
    {
      cerr << "[DataStreamer::processCameraCommand] Invalid setLayerVisibility command" << endl;
      return;
    }

    string layer = d["layer"].GetString();
    bool visible = d["visible"].GetBool();

    if (layer == "blobs")
      d_drawBlobs = visible;
    else if (layer == "observedLines")
      d_drawObservedLines = visible;
    else if (layer == "expectedLines")
      d_drawExpectedLines = visible;
    else
      cerr << "[DataStreamer::processCameraCommand] Invalid setLayerVisibility command. Unknown layer: " << layer << endl;
  }
}