#include "agent.ih"

bool Agent::init()
{
  cout << "[Agent::init] Start" << endl;

  // Check if camera is opened successfully
  if (!d_camera.isOpened())
  {
    cout << "[Agent::init] Failed to open camera!" << endl;
    return false;
  }

  if (d_showUI)
  {
    cv::namedWindow("raw");
    cv::namedWindow("labeled");
  }

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
  Robot::Action::GetInstance()->m_Joint.SetEnableBody(true, true);
  Robot::Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);
  Robot::Walking::GetInstance()->m_Joint.SetEnableBodyWithoutHead(true, true);

  cout << "[Agent::init] Enabling motion manager" << endl;
  MotionManager::GetInstance()->SetEnable(true);

  cout << "[Agent::init] Loading actions from motion file: " << d_motionFile.c_str() << endl;
  Robot::Action::GetInstance()->LoadFile((char*)d_motionFile.c_str());

  cout << "[Agent::init] Adopting standing pose" << endl;
  while(Robot::Action::GetInstance()->Start(15) == false) usleep(8000);
  while(Robot::Action::GetInstance()->IsRunning()) usleep(8*1000);

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

  // Build LUT
  cout << "[Agent::init] Building LUT" << endl;
  LUTBuilder lutBuilder;

  vector<hsvRange> ranges;

  // Hardcoded for now
  hsvRange goalRange;
  goalRange.h = 40;
  goalRange.hRange = 10;
  goalRange.s = 210;
  goalRange.sRange = 55;
  goalRange.v = 190;
  goalRange.vRange = 65;
  ranges.push_back(goalRange);

  hsvRange ballRange;
  ballRange.h = 10;
  ballRange.hRange = 15;
  ballRange.s = 255;
  ballRange.sRange = 95;
  ballRange.v = 190;
  ballRange.vRange = 95;
  ranges.push_back(ballRange);

  d_LUT = lutBuilder.buildBGRFromHSVRanges(ranges);

  cout << "[Agent::init] Done" << endl;

  return true;
}
