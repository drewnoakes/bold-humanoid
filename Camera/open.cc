#include "camera.ih"

void Camera::open()
{
  d_fd = ::open(d_device.c_str(), O_RDWR);

  if (d_fd < 0)
  {
    exit(0);
    log::error("Camera::open") << "Failed opening device: " << d_device;
  }

  log::info("Camera::open") << "Camera opened";

  // List capabilities
  ioctl(d_fd, VIDIOC_QUERYCAP, &d_capabilities);

  // List current format
  v4l2_format formatReq;
  memset(&formatReq, 0, sizeof(formatReq));
  formatReq.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ioctl(d_fd, VIDIOC_G_FMT, &formatReq);
  d_pixelFormat = PixelFormat(formatReq.fmt.pix);
  d_pixelFormat.owner = this;

  createControls();
  createFormats();
}
