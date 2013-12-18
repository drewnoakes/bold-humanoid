#include "camera.ih"

string fracToString(v4l2_fract f)
{
  stringstream s;
  s << f.numerator << "/" << f.denominator;
  return s.str();
}

void Camera::logFrameIntervalDetails() const
{
  struct v4l2_frmivalenum frmDesc;
  frmDesc.index = 0;
  frmDesc.pixel_format = d_pixelFormat.id;
  frmDesc.width = d_pixelFormat.width;
  frmDesc.height = d_pixelFormat.height;

  while (ioctl(d_fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmDesc) == 0)
  {
    if (frmDesc.type == V4L2_FRMIVAL_TYPE_DISCRETE)
      log::info("Camera::logFrameIntervalDetails") << "Discrete " << fracToString(frmDesc.discrete);
    else if (frmDesc.type == V4L2_FRMIVAL_TYPE_CONTINUOUS)
      log::info("Camera::logFrameIntervalDetails") << "Continuous";
    else if (frmDesc.type == V4L2_FRMIVAL_TYPE_STEPWISE)
      log::info("Camera::logFrameIntervalDetails") << "Stepwise min=" << fracToString(frmDesc.stepwise.min) << " max=" << fracToString(frmDesc.stepwise.max) << " step=" << fracToString(frmDesc.stepwise.step);

    frmDesc.index++;
  }
}
