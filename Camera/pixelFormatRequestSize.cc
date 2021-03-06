#include "camera.ih"

bool Camera::PixelFormat::requestSize(unsigned width, unsigned height)
{
  v4l2_format formatReq;
  memset(&formatReq, 0, sizeof(formatReq));
  formatReq.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  formatReq.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  formatReq.fmt.pix.width = width;
  formatReq.fmt.pix.height = height;
  formatReq.fmt.pix.pixelformat = pixelFormat;
  int res = ioctl(owner->d_fd, VIDIOC_S_FMT, &formatReq);

  if (res < 0)
  {
    log::error("Camera::PixelFormat::requestSize") << "Error setting camera size: " << strerror(errno) << " (" << errno << ")";
  }

  ioctl(owner->d_fd, VIDIOC_G_FMT, &formatReq);

  owner->d_pixelFormat.width = formatReq.fmt.pix.width;
  owner->d_pixelFormat.height = formatReq.fmt.pix.height;
  owner->d_pixelFormat.bytesPerLine = formatReq.fmt.pix.bytesperline;
  owner->d_pixelFormat.imageByteSize = formatReq.fmt.pix.sizeimage;

  return owner->d_pixelFormat.width == width
      && owner->d_pixelFormat.height == height;
}
