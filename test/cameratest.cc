#include "../Camera/camera.hh"
#include <iostream>
#include "../PixelFilterChain/pixelfilterchain.hh"

using namespace bold;
using namespace std;


int main()
{
  Camera cam("/dev/video0");

  cam.open();

  auto controls = cam.getControls();

  cout << "===== CAPABILITIES =====" << endl;

  cout << "Read/write: " << (cam.canRead() ? "YES" : "NO") << endl;
  cout << "Streaming:  " << (cam.canStream() ? "YES" : "NO") << endl;

  cout << "===== CONTROLS =====" << endl;;
  for (auto control : controls)
  {
    if (control.name == "Brightness")
      control.setValue(50);

    cout << "Control: " << control.name << " " << control.type << " (" << control.minimum << "-" << control.maximum << ", def: " << control.defaultValue << "), val.: ";

    cout << control.getValue() << endl;
  }

  cout << "===== FORMATS =====" << endl;
  auto formats = cam.getFormats();
  for (auto format : formats)
  {
    cout << "Format: "  << format.description << endl;
  }

  cout << "===== CURRENT FORMAT =====" << endl;
  cam.getPixelFormat().requestSize(640,400);
  auto pixelFormat = cam.getPixelFormat();


  cout << "Width          : " << pixelFormat.width << endl;
  cout << "Height         : " << pixelFormat.height << endl;
  cout << "Bytes per line : " << pixelFormat.height << endl;
  cout << "Bytes total    : " << pixelFormat.imageByteSize << endl;

  PixelFilterChain chain;

  chain.pushFilter([](unsigned char* pxl) {
      int y = pxl[0] - 16;
      int cb = pxl[1] - 128;
      int cr = pxl[2] - 128;

      int b = (298 * y + 516 * cb + 128) >> 8;
      if (b < 0)
        b = 0;
      int g = (298 * y - 100 * cb - 208 * cr) >> 8;
      if (g < 0)
        g = 0;
      int r = (298 * y + 409 * cr + 128) >> 8;
      if (r < 0)
        r = 0;

      pxl[0] = b;
      pxl[1] = g;
      pxl[2] = r;
    });


  cv::namedWindow("main");

  cam.startCapture();

  for (;;)
  {
    int c = cv::waitKey(1);

    if (c == 'q')
      break;

    cv::Mat img = cam.capture();
    chain.applyFilters(img);
    cv::imshow("main", img);

    cout << "." << endl;
  }

  cam.stopCapture();
}
