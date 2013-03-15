#include "agent.ih"

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

  // TODO only stream if argument specified?
  // TODO port from config, not constructor
  d_streamer = new DataStreamer(8080);
  d_streamer->initialise(d_ini);
  d_streamer->setCamera(d_camera);

  d_streamer->registerControls("camera", d_camera->getControls());
  d_streamer->registerControls("vision", VisualCortex::getInstance().getControls());

  Debugger::getInstance().update(d_CM730);

  AgentModel::getInstance().initialise(d_ini);

  Robot::MotionManager* motionManager = Robot::MotionManager::GetInstance();
  Robot::Walking* walkingModule = Robot::Walking::GetInstance();
  Robot::Action* actionModule = Robot::Action::GetInstance();
  Robot::Head* headModule = Robot::Head::GetInstance();

  //
  // Motion manager
  //
  cout << "[Agent::init] Initialising motion manager" << endl;
  if (!motionManager->Initialize(&d_CM730))
  {
    cout << "[Agent::init] motion manager initialisation FAILED" << endl;
    return false;
  }

  cout << "[Agent::init] Loading walk parameters" << endl;
  walkingModule->LoadINISettings(&d_ini);

  cout << "[Agent::init] Adding motion modules" << endl;
  motionManager->AddModule((MotionModule*)actionModule);
  motionManager->AddModule((MotionModule*)headModule);
  motionManager->AddModule((MotionModule*)walkingModule);

  cout << "[Agent::init] Loading motion manager settings" << endl;
  motionManager->LoadINISettings(&d_ini);

  cout << "[Agent::init] Setup and starting motion timer" << endl;
  d_motionTimer = new LinuxMotionTimer(motionManager);
  d_motionTimer->Start();

  cout << "[Agent::init] Registering motion modules" << endl;
  walkingModule->m_Joint.SetEnableBodyWithoutHead(true, true);

  cout << "[Agent::init] Enabling motion manager" << endl;
  motionManager->SetEnable(true);

  cout << "[Agent::init] Loading actions from motion file: " << d_motionFile.c_str() << endl;
  actionModule->LoadFile((char*)d_motionFile.c_str());

  cout << "[Agent::init] Enable body" << endl;
  actionModule->m_Joint.SetEnableBody(true, true);

//   cout << "[Agent::init] Sit down" << endl;
//   while(actionModule->Start("sit down") == false)
//     usleep(8000);
//   while(actionModule->IsRunning())
//     usleep(8*1000);

  cout << "[Agent::init] Stand up" << endl;
  while(actionModule->Start("stand up") == false)
    usleep(8000);
  while(actionModule->IsRunning())
    usleep(8*1000);

  cout << "[Agent::init] Calibrating gyro & acc..." << endl;
  motionManager->ResetGyroCalibration();
  while(1)
  {
    if(motionManager->GetCalibrationStatus() == 1)
    {
      cout << "[Agent::init] Calibration complete" << endl;
      break;
    }
    else if(motionManager->GetCalibrationStatus() == -1)
    {
      cout << "[Agent::init] Calibration failed" << endl;
      motionManager->ResetGyroCalibration();
    }
    usleep(8000);
  }

  d_state = S_INIT;

  cout << "[Agent::init] Done" << endl;

  return true;
}
