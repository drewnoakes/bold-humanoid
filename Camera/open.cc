#include "camera.ih"

void Camera::open()
{
  d_fd = ::open(d_device.c_str(), O_RDWR);

  if (d_fd < 0)
  {
    cout << "[Camera::open] Failed opening device: " << d_device << endl;
    exit(0);
  }

  cout << "[Camera::open] Camera opened" << endl;

  // List capabilities
  ioctl(d_fd, VIDIOC_QUERYCAP, &d_capabilities);

  // List current format
  v4l2_format formatReq;
  memset(&formatReq, 0, sizeof(formatReq));
  formatReq.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ioctl(d_fd, VIDIOC_G_FMT, &formatReq);
  d_pixelFormat = PixelFormat(formatReq.fmt.pix);
  d_pixelFormat.owner = this;

  // List user controls
  d_controls = listControls();

  // List image formats
  d_formats = listFormats();
}
