#include "agent.ih"

void Agent::init()
{
  cout << "[Agent::init] Start" << endl;

  // Check if camera is opened successfully
  if (!d_camera.isOpened())
  {
    cout << "[Agent::init] Failed to open camera!" << endl;
    return;
  }

  cv::namedWindow("raw");
  cv::namedWindow("labeled");

  // Initialize motion manager
  cout << "[Agent::init] Initialize motion manager" << endl;
  if(MotionManager::GetInstance()->Initialize(&d_CM730) == false)
  {
    cout << "[Agent::init] Failed to initialize Motion Manager!" << endl;
    return;
  }

  // Load walk parameters
  cout << "[Agent::init] Load walk parameters" << endl;
  Walking::GetInstance()->LoadINISettings(&d_ini);

  // Add motion modules
  cout << "[Agent::init] Add motion modules" << endl;
  // Playing action scripts
  MotionManager::GetInstance()->AddModule((MotionModule*)Robot::Action::GetInstance());
  // Moving the head
  MotionManager::GetInstance()->AddModule((MotionModule*)Robot::Head::GetInstance());
  // Walking
  MotionManager::GetInstance()->AddModule((MotionModule*)Robot::Walking::GetInstance());

  // Load motion manager settings
  cout << "[Agent::init] Load motion manager settings" << endl;
  MotionManager::GetInstance()->LoadINISettings(&d_ini);

  // Setup and start motion timer
  cout << "[Agent::init] Setup and start motion timer" << endl;
  d_motionTimer = new LinuxMotionTimer(MotionManager::GetInstance());
  d_motionTimer->Start();

  // Turn on body
  cout << "[Agent::init] Enable body and motion manager" << endl;
  Robot::Action::GetInstance()->m_Joint.SetEnableBodyWithoutHead(true, true);
  // Enable motion mnager
  MotionManager::GetInstance()->SetEnable(true);
  
  // Turn on LED panel on back
  cout << "[Agent::init] Turning on LEDs" << endl;
  d_CM730.WriteByte(CM730::P_LED_PANNEL, 0x01|0x02|0x04, NULL);
  
  // Load motion file
  Robot::Action::GetInstance()->LoadFile((char*)d_motionFile.c_str());

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

  // Get into starting position
  cout << "[Agent::init] Start position" << endl;
  Robot::Action::GetInstance()->Start(15);

  // Wait until we are in starting position
  while(Robot::Action::GetInstance()->IsRunning())
    usleep(8*1000);

  cout << "[Agent::init] Done" << endl;

}
