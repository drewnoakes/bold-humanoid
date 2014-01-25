%{
#include <Setting/setting.hh>
#include <Config/config.hh>
%}

namespace bold
{
  class Config
  {
  public:
    static bold::SettingBase* getSettingBase(std::string path);
    template<typename T>
      static bold::Setting<T>* getSetting(std::string path);
    template<typename T>
      static T getValue(std::string path);
    template<typename T>
      static T getStaticValue(std::string path);

    static void initialise(std::string metadataFile, std::string configFile);
    static void initialisationCompleted();
    static bool isInitialising();
  };
}

%define GETSETTING_TEMPLATE(T,N)
  %template(get ## N ## Setting) bold::Config::getSetting<T >;
  %template(get ## N ## Value) bold::Config::getValue<T >;
  %template(getStatic ## N ## Value) bold::Config::getStaticValue<T >;
%enddef

GETSETTING_TEMPLATE(int,Int);
GETSETTING_TEMPLATE(int,Enum);
GETSETTING_TEMPLATE(double,Double);
GETSETTING_TEMPLATE(bool,Bool);
GETSETTING_TEMPLATE(bold::Colour::hsvRange,HsvRange);
GETSETTING_TEMPLATE(bold::Range<double>,DoubleRange);
GETSETTING_TEMPLATE(std::string,String);
GETSETTING_TEMPLATE(int,Int);

