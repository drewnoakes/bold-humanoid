#include "camera.ih"

int Camera::getControlValue(Control const& control)
{
  // TODO rename this as updateControlValue and return void

//assert(control.getFamily() == "camera");

  v4l2_control ctrl;
  ctrl.id = control.getId();
  ioctl(d_fd, VIDIOC_G_CTRL, &ctrl);
  return ctrl.value;
}
