#include "camera.ih"

void Camera::setControlValue(Control const& control)
{
  v4l2_control ctrl;
  ctrl.id = control.id;
  ctrl.value = control.value;
  ioctl(d_fd, VIDIOC_S_CTRL, &ctrl);
}
