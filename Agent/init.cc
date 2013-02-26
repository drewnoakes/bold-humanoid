#include "agent.ih"

bool Agent::init()
{
  cout << "[Agent::init] Start" << endl;

  initCamera();

  d_pfChain.pushFilter([](unsigned char* pxl) {
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

  // Check if camera is opened successfully
  /*
  if (!d_camera.isOpened())
  {
    cout << "[Agent::init] Failed to open camera!" << endl;
    return false;
  }
  */

  if (d_showUI)
  {
    cv::namedWindow("raw");
    cv::namedWindow("labelled");
  }

  // TODO only stream if argument specified?
  d_streamer = new DataStreamer(8080);
  d_streamer->init();

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

  /*
  cout << "[Agent::init] Stand up" << endl;
  while(Robot::Action::GetInstance()->Start("stand up") == false)
    usleep(8000);
  while(Robot::Action::GetInstance()->IsRunning())
    usleep(8*1000);
  */

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
