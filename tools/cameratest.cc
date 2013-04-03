#include <iostream>

#include "../Camera/camera.hh"
#include "../Colour/colour.hh"
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
  for (Control const& control : controls)
    cout << "Control: " << control << endl;

  cout << "===== FORMATS =====" << endl;
  auto formats = cam.getFormats();
  for (auto format : formats)
    cout << "Format: "  << format.description << endl;

  cout << "===== CURRENT FORMAT =====" << endl;
  cam.getPixelFormat().requestSize(640,400);
  auto pixelFormat = cam.getPixelFormat();

  cout << "Width          : " << pixelFormat.width << endl;
  cout << "Height         : " << pixelFormat.height << endl;
  cout << "Bytes per line : " << pixelFormat.height << endl;
  cout << "Bytes total    : " << pixelFormat.imageByteSize << endl;

  PixelFilterChain chain;

  chain.pushFilter(&Colour::yCbCrToBgrInPlace);

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
  }

  cam.stopCapture();
}
