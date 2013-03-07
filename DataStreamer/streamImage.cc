#include "datastreamer.ih"

void DataStreamer::streamImage(cv::Mat const& img, string const& label)
{
  auto streamPtr = find_if(d_imgStreams.begin(), d_imgStreams.end(),
			   [&label] (ImgStream& stream) {
			     return stream.label == label;
			   });

  
  if (streamPtr == d_imgStreams.end())
  {
    ImgStream stream;
    stream.id = d_imgStreams.size();
    stream.label = label;
    
    streamPtr = d_imgStreams.insert(streamPtr, stream);
  }
  
  streamPtr->img = img;

  for (auto ses : d_cameraSessions)
    if (ses->streamSelection == streamPtr->id && !ses->imgSending)
      ses->imgReady = true;

}
