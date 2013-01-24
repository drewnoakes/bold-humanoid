#include "../BlobDetector/blobdetector.hh"

#include <iostream>
#include <Eigen/Eigenvalues>
#include <fstream>

//#include <gtkmm.h>

using namespace std;
using namespace bold;
using namespace Eigen;

/*
Gtk::Main* gtkKit;
      
Gtk::Window* gtkWindow;

Gtk::Scale* gtkHueScale;
*/

bool trackbarsChanged = true;

struct bgr
{
  bgr(int _b, int _g, int _r)
    : b(_b), g(_g), r(_r)
  {}

  unsigned char b;
  unsigned char g;
  unsigned char r;
};

struct hsv
{
  int h;
  int s;
  int v;
};

struct hsvRange
{
  int h;
  int hRange;
  int s;
  int sRange;
  int v;
  int vRange;
};

hsv bgr2hsv(bgr const& in)
{
  hsv         out;
  int         min, max, delta;

  min = in.r < in.g ? in.r : in.g;
  min = min  < in.b ? min  : in.b;

  max = in.r > in.g ? in.r : in.g;
  max = max  > in.b ? max  : in.b;

  out.v = max;                                // v
  delta = max - min;
  if( max > 0 ) {
    out.s = ((delta << 8) / max);                  // s
  } else {
    // r = g = b = 0                        // s = 0, v is undefined
    out.s = 0;
    out.h = 0;
    out.v = 0;// its now undefined
    return out;
  }
  if (delta ==0)
  {
    out.h = 0;
    return out;
  }

  // 0-63, 64-127, 128- 191
  if( in.r == max )                           // > is bogus, just keeps compilor happy
    out.h = (32 + (in.g - in.b) << 5) / delta;        // between yellow & magenta
  else
    if( in.g == max )
      out.h = 96 + ((in.b - in.r) << 5) / delta;  // between cyan & yellow
    else
      out.h = 160 + ((in.r - in.g) << 5) / delta;  // between magenta & cyan

  return out;
}

void makeLUT(char *bgr2lab, int hue, int hrange, int sat, int srange, int val, int vrange)
{
  char* p = bgr2lab;
  for (int b = 0; b < 256; ++b)
    for (int g = 0; g < 256; ++g)
      for (int r = 0; r < 256; ++r)
        {
          hsv hsv = bgr2hsv(bgr(b, g, r));

          // test h
          int diff = abs((int)hsv.h - hue);
          diff = min(diff, 192 - diff);

          if (diff <= hrange &&
              hsv.s >= sat - srange && hsv.s <= sat + srange &&
              hsv.v >= val - vrange && hsv.v <= val + vrange)
            *p = 1;
          else
            *p = 0;

          ++p;
        }
}

void trackbarCallback(int pos, void* userData)
{
  trackbarsChanged = true;
}

hsvRange loadHSVRange(std::string const& file)
{
  ifstream in(file.c_str());
  hsvRange range;
  in >>
    range.h >> range.hRange >>
    range.s >> range.sRange >>
    range.v >> range.vRange;

  return range;
}

int main(int argc, char** argv)
{

  cv::VideoCapture cap(0);
  if (!cap.isOpened())
  {
    cout << "Failed opening camera" << endl;
    return -1;
  }

  /*
  gtkKit = new Gtk::Main(0, 0);

  // Create GUI and gather widgets
  Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file("blobtest.glade");
  gtk_builder_connect_signals(builder->gobj(), 0);
  
  builder->get_widget("mainWindow", gtkWindow);
  builder->get_widget("hueScale", gtkHueScale);
  gtkHueScale->set_range(0,360);

  Gtk::Main::run(*gtkWindow);
  */
  
  if (argc < 2)
  {
    cout << "Usage: " << argv[0] << " FILE" << endl;
    return -1;
  }

  // Create windows
  cv::namedWindow("labeled");
  cv::namedWindow("trackbars");
  cv::namedWindow("main");

  // Load ranges;
  hsvRange range = loadHSVRange("ranges.dat");

  // Add trackbars
  cv::createTrackbar("hue", "trackbars", &range.h, 180, &trackbarCallback);
  cv::createTrackbar("hue_range", "trackbars", &range.hRange, 90, &trackbarCallback);
  cv::createTrackbar("sat", "trackbars", &range.s, 255, &trackbarCallback);
  cv::createTrackbar("sat_range", "trackbars", &range.sRange, 128, &trackbarCallback);
  cv::createTrackbar("val", "trackbars", &range.v, 255, &trackbarCallback);
  cv::createTrackbar("val_range", "trackbars", &range.vRange, 128, &trackbarCallback);

  string file = argv[1];
  bool capture = file == "-";

  cv::Mat image;
  if (!capture)
  {
    cout << "Reading image: " << file << endl;
    image = cv::imread(file, CV_LOAD_IMAGE_COLOR);
  
    if(!image.data )                              // Check for invalid input
    {
      cout <<  "Could not open or find the image" << std::endl ;
      return -1;
    }
  }
  else
  {
    cap >> image;
  }

  // Big ass lookup table
  char *bgr2lab = new char[256*256*256];
  memset(bgr2lab, 0, 256*256*256);
  
  // The labeled image
  cv::Mat labeled(image.rows, image.cols, CV_8UC1);

  cv::imshow("main", image);

  bool shouldProcess = true;

  while (true)
  {
    if (capture)
    {
      int c = cv::waitKey(1);

      if (c == 'q')

      break;
      cap >> image;

      shouldProcess = true;
    }
    else
    {
      int c = cv::waitKey(30);

      if (c == 'q')
	break;
    }

    if (trackbarsChanged)
    {
      cout << "Building LUT..." << endl;

      double t1 = (double)cv::getTickCount();
      
      makeLUT(bgr2lab, range.h, range.hRange, range.s, range.sRange, range.v, range.vRange);

      double t2 = (double)cv::getTickCount();

      cout << "Done! (" << (t2 - t1)  / cv::getTickFrequency() << "s)" << endl;

      shouldProcess = true;
      trackbarsChanged = false;
    }

    if (shouldProcess)
    {
      cout << "Labelling..." << endl;
      double t3 = (double)cv::getTickCount();

      for (unsigned y = 0; y < image.rows; ++y)
      {
        unsigned char *origpix = image.ptr<unsigned char>(y);
        unsigned char *labeledpix = labeled.ptr<unsigned char>(y);
        for (unsigned x = 0; x < image.cols; ++x)
        {
          unsigned char l = bgr2lab[(origpix[0] << 16) | (origpix[1] << 8) | origpix[2]];
          *labeledpix = l;

          ++origpix;
          ++origpix;
          ++origpix;
          ++labeledpix;
        }
      }

      double t4 = (double)cv::getTickCount();

      cout << "Done! (" << (t4 - t3)  / cv::getTickFrequency() << "s)" << endl << "Detecting blobs..." << endl;

      double t5 = (double)cv::getTickCount();

      BlobDetector detector;
      vector<set<Blob> > blobs = detector.detectBlobs(labeled, 1);

      double t6 = (double)cv::getTickCount();

      cout << "Done! (" << (t6 - t5) / cv::getTickFrequency() << "s)" << endl;

      cout << "nr blobs: " << blobs[0].size() << endl;

      // Paint blobs
      cv::Mat marked = image.clone();
      for (Blob const& b : blobs[0])
      {
	// Only of decent size
        if ((b.br - b.ul).minCoeff() > 5 )
        {
	  // Bounding rectangle
          cv::rectangle(marked,
                        cv::Rect(b.ul.x(), b.ul.y(),
                                 b.br.x() - b.ul.x(), b.br.y() - b.ul.y()),
                        cv::Scalar(255,255,0),
			2);
          
	  // Determine orientation
          cout << "covar: " << endl << b.covar << endl;
          
          auto solver = EigenSolver<Matrix2f>(b.covar.cast<float>());
          auto eigenVectors = solver.eigenvectors().real();
          auto eigenValues = solver.eigenvalues().real();

          Vector2f ev1 = b.mean.cast<float>() + eigenVectors.col(0) * sqrt(eigenValues(0));
          Vector2f ev2 = b.mean.cast<float>() + eigenVectors.col(1) * sqrt(eigenValues(1));

          cv::line(marked,
                   cv::Point(b.mean.x(), b.mean.y()),
                   cv::Point(ev1.x(), ev1.y()),
                   cv::Scalar(0,255,0),
		   2);
          cv::line(marked,
                   cv::Point(b.mean.x(), b.mean.y()),
                   cv::Point(ev2.x(), ev2.y()),
                   cv::Scalar(0,255,0),
		   2);
          
          cout << "ev: " << endl << solver.eigenvectors() << endl << "-" << endl << solver.eigenvalues() << endl;

	  // Draw top-most run
	  Run const& topRun = *b.runs.begin();
	  cv::circle(marked,
		     cv::Point((topRun.start.x() + topRun.end.x()) / 2,
			       topRun.start.y()),
		     5,
		     cv::Scalar(255,0,0),
		     2);

        }
      }

      cv::normalize(labeled, labeled, 0, 255, CV_MINMAX );
      cv::imshow("labeled", labeled);
      cv::imshow("main", marked);
      
      shouldProcess = false;
    }
  }

  return 0;
}
