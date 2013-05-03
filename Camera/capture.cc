#include "camera.ih"

Mat Camera::capture()
{
  v4l2_buffer buf;
  memset(&buf, 0, sizeof(buf));

  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;

  if (-1 == ioctl(d_fd, VIDIOC_DQBUF, &buf))
  {
    cout << "[Camera] Error dequeueing buffer" << endl;
    exit(-1);
  }

  if (-1 == ioctl(d_fd, VIDIOC_QBUF, &buf))
  {
    cout << "[Camera] Error re-queueing buffer" << endl;
    exit(-1);
  }

  Mat img(d_pixelFormat.height, d_squash ? d_pixelFormat.width / 2 : d_pixelFormat.width, CV_8UC3);

  unsigned char* datCursor = d_buffers[buf.index].start;
  unsigned char* datEnd = datCursor + 4 * d_pixelFormat.height * d_pixelFormat.width / 2;
  unsigned char* imgCursor = img.data;

  if (d_squash)
  {
    while (datCursor != datEnd)
    {
      int y1 = datCursor[0];
      int y2 = datCursor[2];
      int cb = datCursor[1];
      int cr = datCursor[3];

      imgCursor[0] = y1 + y2 / 2;
      imgCursor[1] = cb;
      imgCursor[2] = cr;

      imgCursor += 3;
      datCursor += 4;
    }
  }
  else
  {
    while (datCursor != datEnd)
    {
      int y1 = datCursor[0];
      int y2 = datCursor[2];
      int cb = datCursor[1];
      int cr = datCursor[3];

      imgCursor[0] = y1;
      imgCursor[1] = cb;
      imgCursor[2] = cr;

      imgCursor[3] = y2;
      imgCursor[4] = cb;
      imgCursor[5] = cr;

      imgCursor += 6;
      datCursor += 4;
    }
  }

  return img;
}
