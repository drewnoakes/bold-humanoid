#include "setting-implementations.hh"

#include "../Config/config.hh"

using namespace bold;
using namespace std;

bool SettingBase::isInitialising()
{
  return Config::isInitialising();
}
