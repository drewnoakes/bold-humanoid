#include "camera.ih"
#include <sys/mman.h>

void Camera::initMemoryMapping()
{
  struct v4l2_requestbuffers req;
  memset(&req, 0, sizeof(req));

  // Request 4 buffers
  req.count = 4;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  ioctl(d_fd, VIDIOC_REQBUFS, &req);

  // Didn' t get more than 1 buffers
  if (req.count < 2)
  {
    log::info("Camera") << "Insufficient buffer memory";
    exit(-1);
  }

  d_buffers = vector<Buffer>(req.count);;

  for (unsigned i = 0; i < req.count; ++i)
  {
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));

    buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory      = V4L2_MEMORY_MMAP;
    buf.index       = i;

    ioctl(d_fd, VIDIOC_QUERYBUF, &buf);

    d_buffers[i].index = buf.index;
    d_buffers[i].length = buf.length;
    d_buffers[i].start =
      (unsigned char*)mmap(NULL /* start anywhere */,
                           buf.length,
                           PROT_READ | PROT_WRITE /* required */,
                           MAP_SHARED /* recommended */,
                           d_fd, buf.m.offset);

    if (MAP_FAILED == d_buffers[i].start)
    {
      log::info("Camera") << "Failed mapping device memory";
      exit(-1);
    }
  }
}
