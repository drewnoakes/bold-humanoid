#include "datastreamer.ih"

using namespace rapidjson;

void DataStreamer::sendCameraStateAndOptions(libwebsocket* wsi)
{
  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);

  writer.StartObject();
  {
    //
    // IMAGE TYPES
    //
    writer.String("imageTypes");
    writer.StartArray();
    {
      auto addImageType = [&writer,this](ImageType imageType, string name)
      {
        writer.StartObject();
        writer.String("id");
        writer.Uint((unsigned)imageType);
        writer.String("name");
        writer.String(name.c_str(), name.length(), false);
        writer.String("isSelected");
        writer.Bool(imageType == d_imageType);
        writer.EndObject();
      };
      addImageType(ImageType::RGB, "RGB");
      addImageType(ImageType::Cartoon, "Cartoon");
      addImageType(ImageType::YCbCr, "YCbCr");
      addImageType(ImageType::None, "None");
    }
    writer.EndArray();

    //
    // FRAME PERIOD
    //
    writer.String("streamFramePeriod");
    writer.Uint(d_streamFramePeriod);

    //
    // LAYERS
    //
    writer.String("layers");
    writer.StartArray();
    {
      auto addLayer = [&writer,this](string name, bool isSelected)
      {
        writer.StartObject();
        writer.String("name");
        writer.String(name.c_str(), name.length(), false);
        writer.String("isSelected");
        writer.Bool(isSelected);
        writer.EndObject();
      };
      addLayer("Blobs", d_drawBlobs);
      addLayer("Lines (observed)", d_drawObservedLines);
      addLayer("Lines (expected)", d_drawExpectedLines);
    }
    writer.EndArray();

    //
    // CONTROLS
    //
    writer.String("controls");
    writer.StartArray();
    {
      for (auto& control : d_camera->getControls())
      {
        writer.StartObject();
        {
          writer.String("id");
          writer.Uint(control.id);

          writer.String("type");
          writer.Int((unsigned)control.type);

          writer.String("name");
          writer.String(control.name.c_str());

          writer.String("minimum");
          writer.Int(control.minimum);

          writer.String("maximum");
          writer.Int(control.maximum);

          writer.String("step");
          writer.Int(control.step);

          writer.String("value");
          writer.Int(control.getValue());

          writer.String("defaultValue");
          writer.Int(control.defaultValue);

          if (control.type == Camera::CT_MENU)
          {
            writer.String("menuItems");
            writer.StartArray();
            {
              for (auto& menuItem : control.menuItems)
              {
                writer.StartObject();
                {
                  writer.String("id");
                  writer.Uint(menuItem.id);

                  writer.String("index");
                  writer.Uint(menuItem.index);

                  writer.String("name");
                  writer.String(menuItem.name.c_str());
                }
                writer.EndObject();
              }
            }
            writer.EndArray();
          }
        }
        writer.EndObject();
      }
    }
    writer.EndArray();
  }
  writer.EndObject();

  const char* json = buffer.GetString();

  cout << "[DataStreamer::sendCameraStateAndOptions] sending: " << json << endl;

  unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING +
                    buffer.GetSize() +
                    LWS_SEND_BUFFER_POST_PADDING];
  unsigned char* p = buf + LWS_SEND_BUFFER_POST_PADDING;
  memcpy(p, buffer.GetString(), buffer.GetSize());

  int res = libwebsocket_write(wsi, p, buffer.GetSize(), LWS_WRITE_TEXT);
}
