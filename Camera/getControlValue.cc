#include "camera.ih"

void Camera::getControlValue(Control& control)
{
  v4l2_control ctrl;
  ctrl.id = control.id;
  ioctl(d_fd, VIDIOC_G_CTRL, &ctrl);
  control.value = ctrl.value;
}
