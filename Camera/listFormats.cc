#include "camera.ih"

vector<Camera::Format> Camera::listFormats()
{
  vector<Format> formats;

  struct v4l2_fmtdesc fmtDesc;
  fmtDesc.index = 0;
  fmtDesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  
  while (ioctl(d_fd, VIDIOC_ENUM_FMT, &fmtDesc) == 0)
  {
    formats.push_back(Format(fmtDesc));
    fmtDesc.index++;
  }
  
  return formats;
}
