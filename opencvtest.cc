#include <opencv.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <utility>

#include <algorithm>
#include <sstream>

using namespace cv;
using namespace std;

struct Blob
{
  Blob(int _id, int _x1, int _y1, int _x2, int _y2)
    : id(_id), x1(_x1), y1(_y1), x2(_x2), y2(_y2)
  {
    root = this;
  }

  int id;
  Blob *root;

  int x1;
  int x2;
  int y1;
  int y2;

  int area() const { return (x2 - x1) * (y2 - y1); }
};

struct Run
{
  Run(int _x1, int _y)
    : x1(_x1), y(_y), x2(x1), length(1), blob(0)
  {}

  int x1;
  int x2;
  int y;
  int length;

  Blob *blob;
};

vector<pair<Blob*, Blob*> > equivs;

vector<Blob*> blobs;
vector<vector<Run *> > runs;

Blob *findRoot(Blob *b)
{
  while (b->root != b)
    b = b->root;
  return b;
}

Blob *unionBlobs(Blob *a, Blob *b)
{
  Blob *ra = findRoot(a);
  Blob *rb = findRoot(b);

  Blob *p;
  Blob *c;
  if (ra->id < rb->id)
  {
    p = ra;
    c = rb;
  }
  else
  {
    p = rb;
    c = ra;
  }

  c->root = p;
  
  p->x1 = min(p->x1, c->x1);
  p->y1 = min(p->y1, c->y1);
  p->x2 = max(p->x2, c->x2);
  p->y2 = max(p->y2, c->y2);
  
  return p;
}

void findBlobs(Mat const& img, Mat& dst)
{
  Run *curRun = 0;
  int runCnt = 0;

  // Find runs
  for (unsigned y = 0; y < img.rows; ++y)
  {
    vector<Run*> rowruns;

    unsigned char const* row = img.ptr<unsigned char>(y);
    for (unsigned x = 0; x < img.cols; ++x)
    {
      if (row[x] != 0)
      {
	if (curRun == 0)
	  curRun = new Run(x, y);
      }
      else // row[x] == 0
      {
	if (curRun != 0)
	{
	  // Finished run
	  curRun->x2 = x;
	  curRun->length = x - curRun->x1;

	  rowruns.push_back(curRun);
	  curRun = 0;
	} 
      } 
    }
    
    if (curRun != 0)
    {
      curRun->x2 = img.cols;
      curRun->length = img.cols - curRun->x1;

      rowruns.push_back(curRun);
      curRun = 0;
    }

    runs.push_back(rowruns);
  }

  cout << " runs found: " << runCnt << endl;

  // Connect runs
  for (int y = 0; y < img.rows; ++y)
  {
    for_each(runs[y].begin(), runs[y].end(), [&](Run *curRun) {
	// Find connected parent runs
	curRun->blob = 0;
	if (y > 0)
	{
	  vector<Run*> prefrowruns = runs[y - 1];
	  for_each(prefrowruns.begin(), prefrowruns.end(), [&](Run *pCand) {
	      // Check for overlap: distance from begin of one to
	      // end of other should be smaller than the sum of
	      // their lengths.
	      int l3 = max(curRun->x2, pCand->x2) - min(curRun->x1, pCand->x1);
	      if (l3 < curRun->length + pCand->length)
	      {
		// Connected!
		Blob *b;
		if (curRun->blob == 0)
		  b = findRoot(pCand->blob);
		else
		  b = unionBlobs(curRun->blob, pCand->blob);

		b->x1 = min(b->x1, curRun->x1);
		b->x2 = max(b->x2, curRun->x2);
		b->y2 = y;
		curRun->blob = b;
	      }
	    });
	}
	if (curRun->blob == 0)
	{
	  Blob* b = new Blob(blobs.size(), curRun->x1, y, curRun->x2, y+1);
	  curRun->blob = b;
	  blobs.push_back(b);
	}
      });
  }
  
  // Filter blobs
  vector<Blob *> largeBlobs;
  int minArea = 100;
  for_each(blobs.begin(), blobs.end(), [&] (Blob *blob) {
      if (blob->root == blob && blob->area() > minArea)
	largeBlobs.push_back(blob);
    });

  cout << "Blobs found: " << largeBlobs.size() << endl;

  // draw blobs
  for_each(largeBlobs.begin(), largeBlobs.end(), [&](Blob *blob) {
      rectangle(dst, Rect(blob->x1, blob->y1, blob->x2 - blob->x1, blob->y2 - blob->y1), Scalar(0,255,0));
    });

  // Clean up
  for_each(blobs.begin(), blobs.end(), [&](Blob *blob) {
      delete blob;
    });

  for_each(runs.begin(), runs.end(), [](vector<Run*>& rs) {
      for_each(rs.begin(), rs.end(), [](Run* r) {
	  delete r;
	});
    });

  blobs.clear();
  runs.clear();
}

struct bgr
{
  bgr(unsigned char _b, unsigned char _g, unsigned char _r)
    : b(_b), g(_g), r(_r)
  {}

  unsigned char b;
  unsigned char g;
  unsigned char r;
};

struct hsv
{
  unsigned char h;
  unsigned char s;
  unsigned char v;
};

hsv bgr2hsv(bgr const& in)
{
  hsv         out;
  unsigned    min, max, delta;

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
  memset(bgr2lab, 0, 255*255*255);
  char* p = bgr2lab;
  for (unsigned b = 0; b < 256; ++b)
    for (unsigned g = 0; g < 256; ++g)
      for (int r = 0; r < 256; ++r)
	{
	  hsv hsv = bgr2hsv(bgr(b, g, r));

	  // test h
	  int diff = abs((int)hsv.h - hue);
	  diff = min(diff, 192 - diff);

	  if (diff <= hrange &&
	      hsv.s >= sat - srange && hsv.s <= sat + srange &&
	      hsv.v >= val - vrange && hsv.v <= val + vrange)
	    *p = 255;

	  ++p;
	}
}

int main() {
  // Create video capture object with device=0, ie default camera
  VideoCapture cap(0);
  if (!cap.isOpened())
    return -1;

  Mat edges;
  
  // Create a named window
  namedWindow("main", 1);
  namedWindow("thres", 1);

  // Add trackbars
  int hue = 0;
  createTrackbar("hue", "thres", &hue, 180);
  int hrange = 10;
  createTrackbar("hue_range", "thres", &hrange, 90);
  int sat = 210;
  createTrackbar("sat", "thres", &sat, 255);
  int srange = 45;
  createTrackbar("sat_range", "thres", &srange, 128);
  int val = 190;
  createTrackbar("val", "thres", &val, 255);
  int vrange = 65;
  createTrackbar("val_range", "thres", &vrange, 128);

  Mat hsv[3];
  Mat hthres;
  Mat frame;
  Mat orig;
  Mat labeled;

  unsigned picCnt = 0;

  char *bgr2lab = new char[256*256*256];
  memset(bgr2lab, 0, 256*256*256);

  // Create lookup table

  makeLUT(bgr2lab, hue, hrange, sat, srange, val, vrange);
  
  for (;;)
  {
  
    cap >> orig;
    labeled = Mat(orig.rows, orig.cols, CV_8UC1);
    double t1 = (double)getTickCount();

    for (unsigned y = 0; y < orig.rows; ++y)
    {
      unsigned char *origpix = orig.ptr<unsigned char>(y);
      unsigned char *labeledpix = labeled.ptr<unsigned char>(y);
      for (unsigned x = 0; x < orig.cols; ++x)
      {
	unsigned char l = bgr2lab[(origpix[0] << 16) | (origpix[1] << 8) | origpix[2]];
	*labeledpix = l;

	++origpix;
	++origpix;
	++origpix;
	++labeledpix;
      }
    }

    double t2 = (double)getTickCount();

    /*
    // Convert to HSV

    // Copy hue channel

    cout << orig.cols << "x" << orig.rows << endl;
    //GaussianBlur(orig, frame, Size(3,3), 1.5, 1.5);
    cvtColor(orig, frame, CV_BGR2HSV);

    double t2 = (double)getTickCount();

    //Canny(frame, frame, 0, 30, 3);
    //Sobel(frame, frame, -1, 1, 1);

    split(frame, hsv);

    // threshold hue channel
    int maxhue = 0;
    for (unsigned y = 0; y < hsv[0].rows; ++y)
    {
      unsigned char *row = hsv[0].ptr<unsigned char>(y);
      for (unsigned x = 0; x < hsv[0].cols; ++x)
      {
	int h = row[x];
	if (h > maxhue)
	  maxhue = h;
	int diff = abs(h - hue);
	diff = min(diff, 180 - diff);

	if (diff <= hrange)
	  row[x] = 255;
	else
	  row[x] = 0;
      }
    }


    cout << maxhue << endl;

    threshold(hsv[1], hsv[1], min(sat+srange, 255), 0, THRESH_TOZERO_INV);
    threshold(hsv[1], hsv[1], max(sat-srange, 0), 255, THRESH_BINARY);
    threshold(hsv[2], hsv[2], min(val+vrange, 255), 0, THRESH_TOZERO_INV);
    threshold(hsv[2], hsv[2], max(val-vrange, 0), 255, THRESH_BINARY);

    bitwise_and(hsv[0], hsv[1], hsv[0]);
    bitwise_and(hsv[0], hsv[2], hsv[0]);
    */

    double t3 = (double)getTickCount();

    findBlobs(labeled, orig);

    double t4 = (double)getTickCount();

    cout << "Times passed in seconds: " << (t4 - t1) / getTickFrequency() << "("  << (t2 - t1) / getTickFrequency() << ", " << (t3 - t2) / getTickFrequency() << ", " << (t4 - t3) / getTickFrequency() << ")" << endl;

    imshow("main", orig);
    imshow("thres", labeled);

    int c = waitKey(30);
    if (c == 'p')
    {
      ostringstream imgname;
      imgname << "photo_" << picCnt++ << ".png";
      imwrite(imgname.str(), orig);
    }
    else if (c >= 0)
      break;
  }

  return 0;
}

