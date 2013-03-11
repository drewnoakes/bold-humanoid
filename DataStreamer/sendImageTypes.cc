#include "datastreamer.ih"

using namespace rapidjson;

void DataStreamer::sendImageTypes(libwebsocket* wsi)
{
  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);

  writer.StartArray();

  auto addImageType = [&writer](ImageType imageType, string name)
  {
    writer.StartObject();
    writer.String("id");
    writer.Uint((unsigned)imageType);
    writer.String("label");
    writer.String(name.c_str(), name.length(), false);
    writer.EndObject(2);
  };

  addImageType(ImageType::None, "None");
  addImageType(ImageType::YCbCr, "YCbCr");
  addImageType(ImageType::RGB, "RGB");
  addImageType(ImageType::Cartoon, "Cartoon");

  writer.EndArray();

  const char* json = buffer.GetString();

  cout << "[DataStreamer::sendStreamLabels] sending: " << json << endl;

  unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING +
                    buffer.GetSize() +
                    LWS_SEND_BUFFER_POST_PADDING];
  unsigned char* p = buf + LWS_SEND_BUFFER_POST_PADDING;
  memcpy(p, buffer.GetString(), buffer.GetSize());

  int res = libwebsocket_write(wsi, p, buffer.GetSize(), LWS_WRITE_TEXT);
}
