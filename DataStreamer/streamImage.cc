#include "datastreamer.ih"

void DataStreamer::streamImage(cv::Mat const& img, string const& label)
{
  auto streamIdxPtr = d_imgStreamLabels.find(label);
  unsigned idx;
  if (streamIdxPtr == d_imgStreamLabels.end())
  {
    idx = d_imgStreamLabels[label] = d_imgStreams.size();
    d_imgStreams.push_back(img);
  }
  else
  {
    idx = *streamIdxPtr;
    d_imgStreams[ix] = img;
  }

  // If a client is listening to this stream, fire off a callback
  bool somebodyListening = false;
  cout << "nr sessions: " << d_cameraSessions.size() << endl;
  for (auto ses : d_cameraSessions)
  {
    cout << "got: " << label << ". ses listening to: " << ses->labelSelection << endl;
    if (ses->labelSelection == idx)
    {
      somebodyListening = true;
      ses->imgReady = true;
    }
  }
}
