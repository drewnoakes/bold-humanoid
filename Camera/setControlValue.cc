#include "camera.ih"

void Camera::Control::setValue(int value)
{
  v4l2_control ctrl;
  ctrl.id = id;
  ctrl.value = value;
  ioctl(owner->d_fd, VIDIOC_S_CTRL, &ctrl);
}
