#include "agent.ih"

// TODO: included here to speed up compilation. should put in internal
// header file and add precompilation to CMakeLists
#include "../DataStreamer/datastreamer.hh"

bool Agent::init()
{
  cout << "[Agent::init] Start" << endl;

  initCamera();

  // Check if camera is opened successfully
  /*
  if (!d_camera.isOpened())
  {
    cout << "[Agent::init] Failed to open camera!" << endl;
    return false;
  }
  */

//   if (d_showUI)
//   {
//     cv::namedWindow("raw");
//     cv::namedWindow("labelled");
//   }

  // TODO only stream if argument specified?
  d_streamer = new DataStreamer(8080);
  d_streamer->initialise(d_ini);
  d_streamer->setCamera(d_camera);

  Debugger::getInstance().update(d_CM730);

  //
  // Motion manager
  //
  cout << "[Agent::init] Initialising motion manager" << endl;
  if(MotionManager::GetInstance()->Initialize(&d_CM730) == false)
  {
    cout << "[Agent::init] Failed to initialize Motion Manager!" << endl;
    return false;
  }

  cout << "[Agent::init] Loading walk parameters" << endl;
  Walking::GetInstance()->LoadINISettings(&d_ini);

  cout << "[Agent::init] Adding motion modules" << endl;
  MotionManager::GetInstance()->AddModule((MotionModule*)Robot::Action::GetInstance());
  MotionManager::GetInstance()->AddModule((MotionModule*)Robot::Head::GetInstance());
  MotionManager::GetInstance()->AddModule((MotionModule*)Robot::Walking::GetInstance());

  cout << "[Agent::init] Loading motion manager settings" << endl;
  MotionManager::GetInstance()->LoadINISettings(&d_ini);

  cout << "[Agent::init] Setup and starting motion timer" << endl;
  d_motionTimer = new LinuxMotionTimer(MotionManager::GetInstance());
  d_motionTimer->Start();

  cout << "[Agent::init] Registering motion modules" << endl;
  Robot::Walking::GetInstance()->m_Joint.SetEnableBodyWithoutHead(true, true);

  cout << "[Agent::init] Enabling motion manager" << endl;
  MotionManager::GetInstance()->SetEnable(true);

  cout << "[Agent::init] Loading actions from motion file: " << d_motionFile.c_str() << endl;
  Robot::Action::GetInstance()->LoadFile((char*)d_motionFile.c_str());

  cout << "[Agent::init] Adopting standing pose" << endl;
  Robot::Action::GetInstance()->m_Joint.SetEnableBody(true, true);

  cout << "[Agent::init] Sit down" << endl;
  while(Robot::Action::GetInstance()->Start("sit down") == false)
    usleep(8000);
  while(Robot::Action::GetInstance()->IsRunning())
    usleep(8*1000);

  cout << "[Agent::init] Stand up" << endl;
  while(Robot::Action::GetInstance()->Start("stand up") == false)
    usleep(8000);
  while(Robot::Action::GetInstance()->IsRunning())
    usleep(8*1000);

  cout << "[Agent::init] Calibrating gyro & acc..." << endl;
  MotionManager::GetInstance()->ResetGyroCalibration();
  while(1)
  {
    if(MotionManager::GetInstance()->GetCalibrationStatus() == 1)
    {
      cout << "[Agent::init] Calibration complete" << endl;
      break;
    }
    else if(MotionManager::GetInstance()->GetCalibrationStatus() == -1)
    {
      cout << "[Agent::init] Calibration failed" << endl;
      MotionManager::GetInstance()->ResetGyroCalibration();
    }
    usleep(8000);
  }

  d_state = S_INIT;

  cout << "[Agent::init] Done" << endl;

  return true;
}
