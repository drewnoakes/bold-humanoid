#include "setting-implementations.hh"

#include "../Config/config.hh"

using namespace bold;

bool SettingBase::isInitialising()
{
  return Config::isInitialising();
}
