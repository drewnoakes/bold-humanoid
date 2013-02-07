#include "camera.ih"

vector<Camera::Control> Camera::listControls()
{
  vector<Control> controls;

  struct v4l2_queryctrl queryctrl;  
  
  queryctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
  while (0 == ioctl(d_fd, VIDIOC_QUERYCTRL, &queryctrl))
  {
    if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
      continue;
    
    controls.push_back(Control(queryctrl));

    queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
  }

  return controls;
}
