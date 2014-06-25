#include "camera.ih"

Mat Camera::capture(SequentialTimer& t)
{
  v4l2_buffer buf;
  memset(&buf, 0, sizeof(buf));
  t.timeEvent("Zero Memory");

  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;

  if (-1 == ioctl(d_fd, VIDIOC_DQBUF, &buf))
  {
    log::error("Camera::capture") << "Error dequeueing buffer: " << strerror(errno) << " (" << errno << ")";
    // NOTE saw this during a match when the robot fell and the cable came out of the camera
    exit(-1);
  }
  t.timeEvent("Dequeue");

  // Snapshot some state objects at this moment
  // TODO can buf.timestamp already old at this point? if so, do we need to snapshot an older time?
  State::snapshot(StateTime::CameraImage);
  t.timeEvent("Snapshot State");

  if (-1 == ioctl(d_fd, VIDIOC_QBUF, &buf))
  {
    log::error("Camera::capture") << "Error re-queueing buffer: " << strerror(errno) << " (" << errno << ")";
    exit(-1);
  }
  t.timeEvent("Requeue");

  if (!d_imageFeed.empty())
  {
    log::verbose("Camera::capture") << "Using image feed. " << d_imageFeed.cols << "x" << d_imageFeed.rows;
    return d_imageFeed.clone();
  }

  // TODO VISION measure perf difference of reusing this image memory
  Mat img(d_pixelFormat.height, d_squash ? d_pixelFormat.width / 2 : d_pixelFormat.width, CV_8UC3);

  uchar* datCursor = d_buffers[buf.index].start;
  uchar* datEnd = datCursor + 4 * d_pixelFormat.height * d_pixelFormat.width / 2;
  uchar* imgCursor = img.data;

  // Convert each set of four input values into either one or two pixels
  if (d_squash)
  {
    while (datCursor != datEnd)
    {
      int y1 = datCursor[0];
      int y2 = datCursor[2];
      uchar cb = datCursor[1];
      uchar cr = datCursor[3];

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
      uchar y1 = datCursor[0];
      uchar y2 = datCursor[2];
      uchar cb = datCursor[1];
      uchar cr = datCursor[3];

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

  t.timeEvent("Copy From Buffer");

  return img;
}
