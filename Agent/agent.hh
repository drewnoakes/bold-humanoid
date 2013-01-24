#ifndef AGENT_HH
#define AGENT_HH

#include <LinuxDARwIn.h>
#include <LinuxCM730.h>

#include <opencv.hpp>

#include "../../vision/BlobDetector/blobdetector.hh"

namespace bold
{
  class Agent
  {
  public:
    Agent(std::string const& U2D_dev,
	  std::string const& iniFile,
	  std::string const& motionFile)
      : d_linuxCM730(U2D_dev.c_str()),
	d_CM730(&d_linuxCM730),
	d_ini(iniFile),
	d_motionFile(motionFile),
	d_camera(0)
    {}
    
    void run();
    
  private:
    Robot::LinuxCM730 d_linuxCM730;
    Robot::CM730 d_CM730;
    
    Robot::LinuxMotionTimer* d_motionTimer;
    
    minIni d_ini;
    
    std::string d_motionFile;
    
    cv::VideoCapture d_camera;
    BlobDetector d_blobDetector;

    void init();

    void think();

    void processImage(cv::Mat& image);
  };
}

#endif
