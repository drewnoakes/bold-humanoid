#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <linux/videodev2.h>
#include <opencv2/opencv.hpp>

#include "../util/Maybe.hh"
#include "../Config/config.hh"

namespace bold
{
  typedef unsigned char uchar;
  typedef unsigned int uint;

  class SettingBase;
  class SequentialTimer;

  class Camera
  {
  public:
    enum class V4L2ControlType : uint
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
        : owner(nullptr),
          id(pf.pixelformat),
          width (pf.width),
          height(pf.height),
          pixelFormat(pf.pixelformat),
          bytesPerLine(pf.bytesperline),
          imageByteSize(pf.sizeimage)
      {}

      Camera* owner;

      unsigned id;
      unsigned width;
      unsigned height;
      unsigned pixelFormat;
      unsigned bytesPerLine;
      unsigned imageByteSize;

      bool requestSize(unsigned width, unsigned height);

      std::string pixelFormatString() const
      {
        char chars[5];
        for (unsigned i = 0; i < 4; ++i)
          chars[i] = ((pixelFormat >> (i * 8)) & 0xFF);
        chars[4] = 0;
        return std::string(chars);
      }
    };

    struct Buffer
    {
      unsigned index;
      unsigned char* start;
      size_t length;
    };

    struct Control
    {
      Control(v4l2_queryctrl const& ctrl, std::map<int,std::string> pairs)
      : id(ctrl.id),
        name((const char*)ctrl.name),
        type((V4L2ControlType)ctrl.type),
        minimum(ctrl.minimum),
        maximum(ctrl.maximum),
        defaultValue(ctrl.default_value),
        pairs(pairs)
      {}

      uint id;
      std::string name;
      V4L2ControlType type;
      int minimum;
      int maximum;
      int defaultValue;
      std::map<int,std::string> pairs;
    };

    Camera(std::string const& device);

    void open();

    std::vector<std::shared_ptr<Control const>> getControls() const { return d_controls; }

    std::shared_ptr<Control const> getControl(std::string const& controlName) const
    {
      for (auto c : d_controls)
        if (c->name == controlName)
          return c;

      return nullptr;
    }

    std::vector<Format> getFormats() const { return d_formats; }

    PixelFormat getPixelFormat() const { return d_pixelFormat; }

    bool canRead() {  return d_capabilities.capabilities & V4L2_CAP_READWRITE; }
    bool canStream() { return d_capabilities.capabilities & V4L2_CAP_STREAMING; }

    void startCapture();
    void stopCapture();

    cv::Mat capture(SequentialTimer& timer);

    void setSquashWidth(bool squash) { d_squash = squash; }

    void setAutoWB(bool autowb) {
      Setting<bool>* autowbSetting = Config::getSetting<bool>("camera.settings.auto-wb");
      if (autowbSetting == nullptr)
        return;

      autowbSetting->setValue(autowb);
    }

    void logFrameIntervalDetails() const;

    void setImageFeed(cv::Mat image) { d_imageFeed = image; }

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

    cv::Mat d_imageFeed;
  };
}
