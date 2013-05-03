#include "camera.ih"

Camera::Camera(string const& device, shared_ptr<Debugger> debugger)
  : d_device(device),
    d_fd(0),
    d_debugger(debugger),
    d_squash(false)
{}
