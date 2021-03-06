#include "camera.ih"

void Camera::stopCapture()
{
  unsigned type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == ioctl(d_fd, VIDIOC_STREAMOFF, &type))
  {
    log::error("Camera::stopCapture") << "Failed stream stop";
    exit(EXIT_FAILURE);
  }
}
