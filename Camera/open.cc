#include "camera.ih"

void Camera::open()
{
  d_fd = ::open(d_device.c_str(), O_RDWR);
  if (d_fd < 0)
  {
    cout << "[Camera] Failed opening device: " << d_device << endl;
    exit(0);
  }
}
