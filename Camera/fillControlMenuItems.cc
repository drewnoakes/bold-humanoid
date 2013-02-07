#include "camera.ih"

void Camera::fillControlMenuItems(Control& control)
{
  struct v4l2_querymenu querymenu;
  memset (&querymenu, 0, sizeof (querymenu));
  querymenu.id = queryctrl.id;

  for (querymenu.index = queryctrl.minimum;
       querymenu.index <= queryctrl.maximum;
       querymenu.index++) {
    if (0 == ioctl (fd, VIDIOC_QUERYMENU, &querymenu)) {
      printf ("  %s\n", querymenu.name);
    }
  }}
