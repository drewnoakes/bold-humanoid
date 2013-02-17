#include "camera.ih"

void Camera::startCapture()
{
  for (Buffer buffer : d_buffers)
  {
    v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = buffer.index;

    if (-1 == ioctl(d_fd, VIDIOC_QBUF, &buf))
    {
      cout << "[Camera] Buffer failure on capture start" << endl;
      exit(-1);
    }
  }

  unsigned type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == ioctl(d_fd, VIDIOC_STREAMON, &type))
  {
    cout << "[Camera] Failed stream start" << endl;
    exit(-1);
  }
}
