#pragma once

#include <sstream>
#include <iostream>
#include <cassert>

namespace bold
{
  class ConfImpl
  {
  public:
    ConfImpl() {}
    virtual ~ConfImpl() {}

    virtual bool paramExists(std::string const& path) { return false; }

    virtual std::string getParam(std::string const& path, std::string const& defVal) { return defVal; }
    virtual int getParam(std::string const& path, int defVal) { return defVal; }
    virtual double getParam(std::string const& path, double defVal) { return defVal; }
    virtual bool getParam(std::string const& path, bool defVal) { return defVal; }
  };

  class Configurable
  {
  public:
    Configurable(std::string const& nameSpace)
      : d_nameSpace(nameSpace)
    {}

    static void setConfImpl(ConfImpl* impl) { s_confImpl = impl; }

    static ConfImpl* getConfImpl() { return s_confImpl; }

    template <typename T>
    static T getParam(std::string const& nameSpace, std::string const& path, T const& defVal)
    {
      assert(s_confImpl && "Configuration implementation has not been set.");
      return s_confImpl->getParam(nameSpace + "." + path, defVal);
    }

    template <typename T>
    T getParam(std::string const& path, T const& defVal)
    {
      T val = getParam(d_nameSpace, path, defVal);
      std::cout << "Value found: " << val << std::endl;
      return val;
    }

  protected:
    bool paramExists(std::string const& path)
    {
      assert(s_confImpl && "Configuration implementation has not been set.");
      return s_confImpl->paramExists(d_nameSpace + "." + path);
    }

    std::string d_nameSpace;
                         
    static ConfImpl* s_confImpl;
  };
}
