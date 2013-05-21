%{
#include <Configurable/configurable.hh>
%}

%feature("director") ConfImpl;


namespace bold
{
  class ConfImpl
  {
  public:
    ConfImpl();
    virtual ~ConfImpl();

    %rename(getParamStr) getParam(std::string const&, std::string const&);
    %rename(getParamInt) getParam(std::string const&, int);
    %rename(getParamDbl) getParam(std::string const&, double);
    %rename(getParamBool) getParam(std::string const&, bool);

    virtual bool paramExists(std::string const& path);
    virtual std::string getParam(std::string const& path, std::string const& defVal);
    virtual int getParam(std::string const& path, int defVal);
    virtual double getParam(std::string const& path, double defVal);
    virtual bool getParam(std::string const& path, bool defVal);
  };

  class Configurable
  {
  public:
    static void setConfImpl(ConfImpl* impl);
  };
}
