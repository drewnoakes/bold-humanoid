#include "camera.ih"

void Camera::createFormats()
{
  d_formats.clear();

  struct v4l2_fmtdesc fmtDesc;
  fmtDesc.index = 0;
  fmtDesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  while (ioctl(d_fd, VIDIOC_ENUM_FMT, &fmtDesc) == 0)
  {
    d_formats.push_back(Format(fmtDesc));
    fmtDesc.index++;
  }
}
