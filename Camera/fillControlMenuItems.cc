#include "camera.ih"

void Camera::fillControlMenuItems(Control& control)
{
  /*
  struct v4l2_querymenu querymenu;
  memset (&querymenu, 0, sizeof (querymenu));
  querymenu.id = control.id;

  for (querymenu.index = control.minimum;
       querymenu.index <= control.maximum;
       querymenu.index++) {
    if (0 == ioctl (fd, VIDIOC_QUERYMENU, &querymenu)) {
      printf ("  %s\n", querymenu.name);
    }
  }
  */
}
