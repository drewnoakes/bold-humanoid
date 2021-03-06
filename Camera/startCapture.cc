#include "camera.ih"

#include <string.h>
#include <errno.h>

void Camera::startCapture()
{
  // Start memory mapping
  initMemoryMapping();

  for (Buffer buffer : d_buffers)
  {
    v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = buffer.index;

    if (-1 == ioctl(d_fd, VIDIOC_QBUF, &buf))
    {
      log::error("Camera::startCapture") << "Buffer failure on capture start: " << strerror(errno) << " (" << errno << ")";
      exit(EXIT_FAILURE);
    }
  }

  unsigned type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == ioctl(d_fd, VIDIOC_STREAMON, &type))
  {
    log::error("Camera::startCapture") << "Failed stream start: " << strerror(errno) << " (" << errno << ")";
    exit(EXIT_FAILURE);
  }
}
