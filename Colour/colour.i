%{
#include <Colour/colour.hh>
%}

namespace bold
{
  namespace Colour
  {
    struct bgr
    {
      bgr();
      bgr(uchar b, uchar g, uchar r);
      bgr invert() const;

      bool operator==(bgr const& other) const;

      uchar b;
      uchar g;
      uchar r;
    };

    struct YCbCr
    {
      YCbCr();
      YCbCr(uchar y, uchar cb, uchar cr);

      bool isValid() const;

      Colour::bgr toBgrInt() const;
      Colour::bgr toBgrFloat() const;

      bool operator==(YCbCr const& other) const;

      uchar y;
      uchar cb;
      uchar cr;
    };

    class hsv;
    class bgr;

    hsv bgr2hsv(bgr const& _in);
    bgr hsv2bgr(hsv const& _in);

    struct hsv
    {
      hsv();

      hsv(int h, int s, int v);

      bool operator==(hsv const& other) const;

      bgr toBgr() const;

      uchar h;
      uchar s;
      uchar v;
    };

    struct hsvRange
    {
      uchar hMin;
      uchar hMax;
      uchar sMin;
      uchar sMax;
      uchar vMin;
      uchar vMax;

      hsvRange();

      hsvRange(
        uchar hMin, uchar hMax,
        uchar sMin, uchar sMax,
        uchar vMin, uchar vMax);

      uchar getHMid() const;
      uchar getSMid() const;
      uchar getVMid() const;

      /** Obtain the center colour for the range, in BGR format. */
      Colour::bgr toBgr() const;

      bool contains(hsv const& hsv) const;

      bool isValid() const;

      static hsvRange fromBytes(uchar hMin, uchar hMax, uchar sMin, uchar sMax, uchar vMin, uchar vMax);
      static hsvRange fromDoubles(double hMin, double hMax, double sMin, double sMax, double vMin, double vMax);

      hsvRange withHMin(uchar value) const;
      hsvRange withHMax(uchar value) const;
      hsvRange withSMin(uchar value) const;
      hsvRange withSMax(uchar value) const;
      hsvRange withVMin(uchar value) const;
      hsvRange withVMax(uchar value) const;
    };

    void yCbCrToBgrInPlace(uchar* pxl);
  };
}
