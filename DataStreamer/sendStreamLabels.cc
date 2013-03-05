#include "datastreamer.ih"

void DataStreamer::sendStreamLabels(libwebsocket* wsi)
{
  ostringstream labelsOut;
  labelsOut << "[";
  bool fst = true;
  for (auto stream : d_imgStreams)
  {
    if (!fst)
      labelsOut << ",";
    fst = false;
    labelsOut << "{\"id\":" << stream.id << ",\"label\":\"" << stream.label << "\"}";
  }
  labelsOut << "]";

  string labelsStr = labelsOut.str();
  cout << "[DataStreamer::sendStreamLabels] sending: " << labelsStr << endl;

  unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING +
                    labelsStr.size() +
                    LWS_SEND_BUFFER_POST_PADDING];
  unsigned char* p = buf + LWS_SEND_BUFFER_POST_PADDING;
  memcpy(p, labelsStr.c_str(), labelsStr.size());

  int res = libwebsocket_write(wsi, p, labelsStr.size(), LWS_WRITE_TEXT);

}
