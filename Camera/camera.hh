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

    Camera(std::string const& device);

    void open();

    std::vector<std::shared_ptr<Control const>> getControls() const { return d_controls; }

//     Maybe<Control const> getControl(unsigned const& controlId) const
//     {
//       return getControl([&controlId](Control const& c)
//       {
//         return c.getId() == controlId;
//       });
//     }

    std::shared_ptr<Control const> getControl(std::string const& controlName) const
    {
      return getControl([&controlName](std::shared_ptr<Control const> c)
      {
        assert(c);
        return c->getName() == controlName;
      });
    }

    std::shared_ptr<Control const> getControl(std::function<bool(std::shared_ptr<Control const>)> const& pred) const
    {
      for (auto c : d_controls)
        if (pred(c))
          return c;

      return Maybe<Control const>::empty();
    }

    std::vector<Format> getFormats() const { return d_formats; }

    PixelFormat getPixelFormat() const { return d_pixelFormat; }

    bool canRead() {  return d_capabilities.capabilities & V4L2_CAP_READWRITE; }
    bool canStream() { return d_capabilities.capabilities & V4L2_CAP_STREAMING; }

    void startCapture();
    void stopCapture();

    cv::Mat capture(SequentialTimer& timer);

    void setSquashWidth(bool squash) { d_squash = squash; }

    friend class Control;
    friend struct PixelFormat;

  private:
    void createControls();
    void createFormats();
    void initMemoryMapping();

    std::string d_device;
    int d_fd;

    v4l2_capability d_capabilities;

    std::vector<std::shared_ptr<Control const>> d_controls;
    std::vector<Format> d_formats;
    PixelFormat d_pixelFormat;

    std::vector<Buffer> d_buffers;

    /// If true, the obtained image will have half the width
    bool d_squash;
  };
}
