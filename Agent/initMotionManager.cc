#include "agent.ih"

bool Agent::initMotionManager(minIni const& ini)
{
  MotionManager* motionManager = MotionManager::GetInstance();

  Walking* walkingModule = Walking::GetInstance();
  robotis::Action* actionModule = robotis::Action::GetInstance();
  Head* headModule = Head::GetInstance();

  cout << "[Agent::init] Initialising motion manager" << endl;
  if (!motionManager->Initialize(d_CM730.get()))
  {
    cout << "[Agent::init] Motion manager initialisation FAILED" << endl;
    return false;
  }

  cout << "[Agent::init] Loading walk parameters" << endl;
  walkingModule->LoadINISettings(&ini);

  cout << "[Agent::init] Adding motion modules" << endl;
  motionManager->AddModule((MotionModule*)actionModule);
  motionManager->AddModule((MotionModule*)headModule);
  motionManager->AddModule((MotionModule*)walkingModule);

  cout << "[Agent::init] Loading motion manager settings" << endl;
  motionManager->LoadINISettings(&ini);

  cout << "[Agent::init] Setup and starting motion timer" << endl;
  d_motionTimer = make_shared<LinuxMotionTimer>(motionManager);
  d_motionTimer->Start();

  cout << "[Agent::init] Registering motion modules" << endl;
  walkingModule->m_Joint.SetEnableBodyWithoutHead(true, true);

  cout << "[Agent::init] Enabling motion manager" << endl;
  motionManager->SetEnable(true);

  cout << "[Agent::init] Loading actions from motion file: " << d_motionFile.c_str() << endl;
  actionModule->LoadFile((char*)d_motionFile.c_str());

  cout << "[Agent::init] Enable body" << endl;

  cout << "[Agent::init] Sit down" << endl;
  auto sit = d_optionTree->getOption("sitdownaction");
  while (sit->hasTerminated() == 0.0)
  {
    sit->runPolicy();
    usleep(8000);
  }

  cout << "[Agent::init] Calibrating gyro & acc..." << endl;
  motionManager->ResetGyroCalibration();
  while (1)
  {
    if (motionManager->GetCalibrationStatus() == 1)
    {
      cout << "[Agent::init] Calibration complete" << endl;
      break;
    }
    else if (motionManager->GetCalibrationStatus() == -1)
    {
      cout << "[Agent::init] Calibration failed" << endl;
      motionManager->ResetGyroCalibration();
    }
    usleep(8000);
  }

  return true;
}
