#include "datastreamer.ih"

void DataStreamer::streamImage(cv::Mat const& img)
{
  assert(d_imageType != ImageType::None);

  d_image = img;

//   auto streamPtr = find_if(d_imgStreams.begin(), d_imgStreams.end(),
// 			   [&label] (ImgStream& stream) {
// 			     return stream.label == label;
// 			   });
//
//
//   if (streamPtr == d_imgStreams.end())
//   {
//     ImgStream stream;
//     stream.id = d_imgStreams.size();
//     stream.label = label;
//
//     streamPtr = d_imgStreams.insert(streamPtr, stream);
//   }
//
//   streamPtr->img = img;

  for (auto ses : d_cameraSessions)
    if (!ses->imgSending)
      ses->imgReady = true;
}
