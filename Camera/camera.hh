#ifndef BOLD_CAMERA_HH
#define BOLD_CAMERA_HH

#include <string>
#include <vector>
#include <linux/videodev2.h>
#include <opencv2/opencv.hpp>

namespace bold
{
  class Camera
  {
  public:

    enum ControlType
    {
      CT_INT = 1,
      CT_BOOL,
      CT_MENU,
      CT_INT_MENU,
      CT_BITMASK,
      CT_BUTTON,
      CT_INT64,
      CT_STRING,
      CT_CTRL_CLASS
    };

    struct ControlMenuItem
    {
      unsigned id;
      unsigned index;
      std::string name;
      int value;
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
