#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <linux/videodev2.h>
#include <opencv2/opencv.hpp>

#include "../util/Maybe.hh"
#include "../Control/control.hh"

namespace bold
{
  class SequentialTimer;
  
  class Debugger;

  class Camera
  {
  public:

    enum class V4L2ControlType
    {
      CT_INT        = 1,
      CT_BOOL       = 2,
      CT_MENU       = 3,
      CT_BUTTON     = 4,
      CT_INT64      = 5,
      CT_CTRL_CLASS = 6,
      CT_STRING     = 7,
      CT_BITMASK    = 8
    };

    struct Format
    {
      Format(v4l2_fmtdesc const& fd)
        : index(fd.index),
          type(fd.type),
          flags(fd.flags),
          description((const char*)fd.description),
          pixelFormat(fd.pixelformat)
      {}

      unsigned index;
      unsigned type;
      unsigned flags;
      std::string description;
      unsigned pixelFormat;
    };

    struct PixelFormat
    {
      PixelFormat()
      {}

      PixelFormat(v4l2_pix_format const& pf)
        : width (pf.width),
          height(pf.height),
          pixelFormat(pf.pixelformat),
          bytesPerLine(pf.bytesperline),
          imageByteSize(pf.sizeimage)
      {}

      Camera* owner;

      unsigned width;
      unsigned height;
      unsigned pixelFormat;
      unsigned bytesPerLine;
      unsigned imageByteSize;

      bool requestSize(unsigned width, unsigned height);

    };

    struct Buffer
    {
      unsigned index;
      unsigned char* start;
      size_t length;
    };

    Camera(std::string const& device, std::shared_ptr<Debugger> debugger);

    void open();

    std::vector<Control> getControls() const { return d_controls; }

    Maybe<Control> getControl(unsigned const& controlId) const
    {
      return getControl([&controlId](Control const& c)
      {
        return c.getId() == controlId;
      });
    }

    Maybe<Control> getControl(std::string const& controlName) const
    {
      return getControl([&controlName](Control const& c)
      {
        return c.getName() == controlName;
      });
    }

    Maybe<Control> getControl(std::function<bool(Control const&)> const& pred) const
    {
      for (Control const& c : d_controls)
        if (pred(c))
          return Maybe<Control>(c);

      return Maybe<Control>::empty();
    }

    std::vector<Format> getFormats() const { return d_formats; }

    PixelFormat getPixelFormat() const { return d_pixelFormat; }

    bool canRead() {  return d_capabilities.capabilities & V4L2_CAP_READWRITE; }
    bool canStream() { return d_capabilities.capabilities & V4L2_CAP_STREAMING; }

    void startCapture();
    void stopCapture();

    cv::Mat capture(std::shared_ptr<SequentialTimer> timer);

    void setSquashWidth(bool squash) { d_squash = squash; }

    int getControlValue(Control const& control);

    friend class Control;
    friend struct PixelFormat;

  private:
    std::string d_device;
    int d_fd;

    v4l2_capability d_capabilities;

    std::vector<Control> d_controls;
    std::vector<Format> d_formats;
    PixelFormat d_pixelFormat;

    std::vector<Buffer> d_buffers;
    std::shared_ptr<Debugger> d_debugger;

    /// If true, the obtained image will have half the width
    bool d_squash;

    std::vector<Control> listControls();
    std::vector<Format> listFormats();

    void initMemoryMapping();
  };
}
