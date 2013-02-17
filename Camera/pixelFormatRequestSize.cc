#include "camera.ih"

bool Camera::PixelFormat::requestSize(unsigned width, unsigned height)
{
  v4l2_format formatReq;
  memset(&formatReq, 0, sizeof(formatReq));
  formatReq.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  formatReq.fmt.pix.width = width;
  formatReq.fmt.pix.height = height;
  formatReq.fmt.pix.pixelformat = pixelFormat;
  ioctl(owner->d_fd, VIDIOC_S_FMT, &formatReq);

  ioctl(owner->d_fd, VIDIOC_G_FMT, &formatReq);
  
  owner->d_pixelFormat.width = formatReq.fmt.pix.width;
  owner->d_pixelFormat.height = formatReq.fmt.pix.height;
  owner->d_pixelFormat.bytesPerLine = formatReq.fmt.pix.bytesperline;
  owner->d_pixelFormat.imageByteSize = formatReq.fmt.pix.sizeimage;
}
