#include "camera.ih"

void Camera::open()
{
  d_fd = ::open(d_device.c_str(), O_RDWR);

  if (d_fd < 0)
  {
    log::error("Camera::open") << "Failed opening device " << d_device << ": " << strerror(errno) << " (" << errno << ")";
    exit(EXIT_FAILURE);
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

  log::verbose("Camera::open") << "Capabilities:";
  log::verbose("Camera::open") << "  Read/write: " << (canRead() ? "YES" : "NO");
  log::verbose("Camera::open") << "  Streaming:  " << (canStream() ? "YES" : "NO");

  log::verbose("Camera::open") << "Controls (" << getControls().size() << "):";
  for (shared_ptr<Camera::Control const> control : getControls())
    log::verbose("Camera::open") << "  " << control->name;

  log::verbose("Camera::open") << "Formats (" << getFormats().size() << "):";
  for (Camera::Format const& format : getFormats())
    log::verbose("Camera::open") << "  "  << format.description;

//logFrameIntervalDetails();
}
