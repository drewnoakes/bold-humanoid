#pragma once

#include "../Colour/colour.hh"
#include "../Config/config.hh"

namespace bold
{
  typedef unsigned char uchar;
  typedef unsigned short ushort;

  class LEDControl
  {
  public:
    LEDControl()
    : d_isPanelLedDirty(false),
      d_isEyeDirty(false),
      d_isForeheadDirty(false),
      d_panelLedByte(0),
      d_eyeColourShort(0),
      d_foreheadColourShort(0),
      d_eyeColour(0,0,0),
      d_foreheadColour(0,0,0)
    {}

    bool isDirty() const { return d_isPanelLedDirty || d_isEyeDirty || d_isForeheadDirty; }
    bool isPanelLedDirty() const { return d_isPanelLedDirty; }
    bool isEyeDirty()      const { return d_isEyeDirty; }
    bool isForeheadDirty() const { return d_isForeheadDirty; }

    void clearDirtyFlags()
    {
      d_isPanelLedDirty = false;
      d_isEyeDirty = false;
      d_isForeheadDirty = false;
    }

    bool isRedPanelLedLit()   const { return (d_panelLedByte & 1) != 0; }
    bool isBluePanelLedLit()  const { return (d_panelLedByte & 2) != 0; }
    bool isGreenPanelLedLit() const { return (d_panelLedByte & 4) != 0; }

    Colour::bgr getEyeColour()      const { return d_eyeColour; }
    Colour::bgr getForeheadColour() const { return d_foreheadColour; }

    ushort getEyeColourShort()      const { return d_eyeColourShort; }
    ushort getForeheadColourShort() const { return d_foreheadColourShort; }
    uchar  getPanelLedByte()        const { return d_panelLedByte; }

    void setPanelLedStates(bool red, bool blue, bool green)
    {
      static auto enabledSetting = Config::getSetting<bool>("hardware.leds.enable-panel");

      uchar ledFlags = 0;

      if (enabledSetting->getValue())
      {
        if (red)
          ledFlags |= 1;

        if (blue)
          ledFlags |= 2;

        if (green)
          ledFlags |= 4;
      }

      if (ledFlags != d_panelLedByte)
      {
        d_isPanelLedDirty = true;
        d_panelLedByte = ledFlags;
      }
    }

    void setEyeColour(Colour::bgr const& colour)
    {
      static auto enabledSetting = Config::getSetting<bool>("hardware.leds.enable-eyes");

      ushort shortValue = !enabledSetting->getValue()
        ? 0
        : (colour.r >> 3) |
          ((colour.g >> 3) << 5) |
          ((colour.b >> 3) << 10);

      if (d_eyeColourShort != shortValue)
      {
        d_isEyeDirty = true;
        d_eyeColour = colour;
        d_eyeColourShort = shortValue;
      }
    }

    void setForeheadColour(Colour::bgr const& colour)
    {
      static auto enabledSetting = Config::getSetting<bool>("hardware.leds.enable-forehead");

      ushort shortValue = !enabledSetting->getValue()
        ? 0
        : (colour.r >> 3) |
          ((colour.g >> 3) << 5) |
          ((colour.b >> 3) << 10);

      if (d_foreheadColourShort != shortValue)
      {
        d_isForeheadDirty = true;
        d_foreheadColour = colour;
        d_foreheadColourShort = shortValue;
      }
    }

  private:
    bool d_isPanelLedDirty;
    bool d_isEyeDirty;
    bool d_isForeheadDirty;

    uchar d_panelLedByte;
    ushort d_eyeColourShort;
    ushort d_foreheadColourShort;

    Colour::bgr d_eyeColour;
    Colour::bgr d_foreheadColour;
  };
}
