#ifndef BOLD_CAMERA_HH
#define BOLD_CAMERA_HH

#include <string>
#include <vector>
#include <algorithm>
#include <linux/videodev2.h>
#include <opencv2/opencv.hpp>
#include "../util/Maybe.hh"

namespace bold
{
  class Camera
  {
  public:

    enum ControlType
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

    struct ControlMenuItem
    {
      ControlMenuItem(v4l2_querymenu const& qm)
        : id(qm.id),
          index(qm.index),
          name((const char*)qm.name)
      {}

      unsigned id;
      unsigned index;
      std::string name;
    };

    struct Control
    {
      Control(v4l2_queryctrl const& qc)
        : owner(0),
          id(qc.id),
          type((ControlType)qc.type),
          name((const char*)qc.name),
          minimum(qc.minimum),
          maximum(qc.maximum),
          step(qc.step),
          defaultValue(qc.default_value),
          flags(qc.flags)
      {}

      Camera* owner;

      unsigned id;
      ControlType type;
      std::string name;
      int minimum;
      int maximum;
      int step;
      int defaultValue;
      unsigned flags;
      std::vector<ControlMenuItem> menuItems;

      int getValue();
      void setValue(int value);
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


  public:

    Camera(std::string const& device);

    void open();

    std::vector<Control> getControls() const { return d_controls; }

    Maybe<Control> getControl(unsigned controlId) const
    {
      // this just returns the first control?
      for (auto const& control : d_controls)
        return Maybe<Control>(control);

      return Maybe<Control>::empty();

      /*
      auto control = find_if(d_controls.begin(), d_controls.end(),
                            [&controlId](Camera::Control const& c)
                            {
                              return c.id == controlId;
                            });

      return control == d_controls.end()
        ? Maybe<Control>::empty()
        : Maybe<Control>(*control);
      */
    }

    Maybe<Control> getControl(std::string const& controlName) const
    {
      auto control = find_if(d_controls.begin(), d_controls.end(),
                            [&controlName](Camera::Control const& c)
                            {
                              return c.name == controlName;
                            });

      return control == d_controls.end()
        ? Maybe<Control>::empty()
        : Maybe<Control>(*control);
    }

    std::vector<Format> getFormats() const { return d_formats; }

    PixelFormat getPixelFormat() const { return d_pixelFormat; }

    bool canRead() {  return d_capabilities.capabilities & V4L2_CAP_READWRITE; }
    bool canStream() { return d_capabilities.capabilities & V4L2_CAP_STREAMING; }

    void startCapture();
    void stopCapture();

    cv::Mat capture();

    void setSquashWidth(bool squash) { d_squash = squash; }

    friend class Control;
    friend class PixelFormat;

  private:
    std::string d_device;
    int d_fd;

    v4l2_capability d_capabilities;

    std::vector<Control> d_controls;
    std::vector<Format> d_formats;
    PixelFormat d_pixelFormat;

    std::vector<Buffer> d_buffers;

    bool d_squash;

    std::vector<Control> listControls();
    std::vector<Format> listFormats();

    void fillControlMenuItems(Control& control);

    void initMemoryMapping();
  };
}

#endif
