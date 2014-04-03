#pragma once

#include "../option.hh"
#include "../../util/log.hh"

#include <map>
#include <memory>

namespace bold
{
  template<typename T>
  class DispatchOption : public Option
  {
  public:
    DispatchOption(std::string id, std::function<T()> keyGetter)
    : Option(id, "Dispatch"),
      d_keyGetter(keyGetter)
    {}

    void setOption(T key, std::shared_ptr<Option> option)
    {
      d_optionByKey.insert(make_pair(key, option));
    }

    virtual std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override
    {
      T key = d_keyGetter();
      auto it = d_optionByKey.find(key);
      if (it == d_optionByKey.end())
      {
        log::error("DispatchOption::runPolicy") << "No option available for key " << key;
        return {};
      }
      return { it->second };
    }

  private:
    std::function<T()> d_keyGetter;
    std::map<T,std::shared_ptr<Option>> d_optionByKey;
  };
}
