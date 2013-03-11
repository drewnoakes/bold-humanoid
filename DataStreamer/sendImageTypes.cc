#include "datastreamer.ih"

using namespace rapidjson;

void DataStreamer::sendImageTypes(libwebsocket* wsi)
{
  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);

  writer.StartArray();

  writer.StartObject();
  writer.String("id");
  writer.Uint((unsigned)ImageType::None);
  writer.String("label");
  writer.String((char*)"None", 4, false);
  writer.EndObject(2);

//   d.StartObject();
//   d["id"] = Value((unsigned)ImageType::YCbCr);
//   d["label"] = "YCbCr";
//   d.EndObject();
//
//   d.StartObject();
//   d["id"] = Value((unsigned)ImageType::RGB);
//   d["label"] = "RGB";
//   d.EndObject();
//
//   d.StartObject();
//   d["id"] = Value((unsigned)ImageType::Cartoon);
//   d["label"] = "Cartoon";
//   d.EndObject();

  writer.EndArray();

  const char* json = buffer.GetString();
//  string labelsStr = d.GetString();

  cout << "[DataStreamer::sendStreamLabels] sending: " << json << endl;

  unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING +
                    buffer.GetSize() +
                    LWS_SEND_BUFFER_POST_PADDING];
  unsigned char* p = buf + LWS_SEND_BUFFER_POST_PADDING;
  memcpy(p, buffer.GetString(), buffer.GetSize());

  int res = libwebsocket_write(wsi, p, buffer.GetSize(), LWS_WRITE_TEXT);
}
