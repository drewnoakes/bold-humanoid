#pragma once

#include <sstream>
#include <iostream>

namespace bold
{
  class ConfImpl
  {
  public:
    ConfImpl() {}
    virtual ~ConfImpl() {}
    virtual std::string getParam(std::string const& path, std::string const& defVal) { return defVal; }
    virtual int getParam(std::string const& path, int defVal) { return defVal; }
    virtual double getParam(std::string const& path, double defVal) { return defVal; }
  };

  class Configurable
  {
  public:
    Configurable(std::string const& nameSpace)
      : d_nameSpace(nameSpace)
    {}

    static void setConfImpl(ConfImpl* impl) { std::cout << "hello" << std::endl; s_confImpl = impl; }

  protected:
    template <typename T>
    T getParam(std::string const& path, T const& defVal)
    {
      return s_confImpl->getParam(d_nameSpace + "." + path, defVal);
      
    }

    std::string d_nameSpace;
                         
    static ConfImpl* s_confImpl;
  };
}
