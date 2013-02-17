#include "camera.ih"

int Camera::Control::getValue()
{
  v4l2_control ctrl;
  ctrl.id = id;
  ioctl(owner->d_fd, VIDIOC_G_CTRL, &ctrl);
  return ctrl.value;
}
