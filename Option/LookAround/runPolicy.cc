#include "lookaround.ih"

OptionList LookAround::runPolicy()
{
  // Make an oscillatory movement to search for the ball
  double maxAmpH = 70.0;//d_ini.getd("Head Pan/Tilt", "left_limit", 80.0); 
  double maxAmpV = 15.0;//
  timeval tval;
  gettimeofday(&tval, 0);

  double t = tval.tv_sec + tval.tv_usec / 1e6;

  double periodH = 3.0;
  double periodV = 1.4;
  
  float hAngle = sin(t / periodH * 2.0 * M_PI) * maxAmpH;
  float vAngle = (sin(t / periodV * 2.0 * M_PI) + 1.0) * maxAmpV / 2.0;
  
  Robot::Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);
  Robot::Head::GetInstance()->MoveByAngle(hAngle, vAngle);

  return OptionList();
}
