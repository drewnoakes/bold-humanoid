#include "camera.ih"

void Camera::fillControlMenuItems(Control& control)
{
  cout << "[Camera::fillControlMenuItems] start" << endl;

  struct v4l2_querymenu querymenu;
  memset (&querymenu, 0, sizeof (querymenu));
  querymenu.id = control.id;

  for (querymenu.index = control.minimum;
       querymenu.index <= control.maximum;
       querymenu.index++) {
    if (0 == ioctl (d_fd, VIDIOC_QUERYMENU, &querymenu)) {
      cout << "menu: " << querymenu.name << endl;
      control.menuItems.push_back(ControlMenuItem(querymenu));
    }
  }
}
