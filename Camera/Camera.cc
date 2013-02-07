#include "camera.ih"

Camera::Camera(string const& device)
  : d_device(device),
    d_fd(0)
{
}
