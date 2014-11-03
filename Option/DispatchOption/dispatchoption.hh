#pragma once

#include "../option.hh"
#include "../../util/log.hh"

#include <sstream>
#include <map>
#include <set>
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

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override
    {
      T key = d_keyGetter();

      std::stringstream keyStr;
      keyStr << key;
      writer.String("key");
      writer.String(keyStr.str().c_str());

      auto it = d_optionByKey.find(key);
      if (it == d_optionByKey.end())
      {
        static std::set<T> unknownKeys;
        if (unknownKeys.find(key) == unknownKeys.end())
        {
          log::error("DispatchOption::runPolicy") << "No option available for key " << key;
          unknownKeys.insert(key);
        }
        writer.String("found");
        writer.Bool(false);
        return {};
      }

      writer.String("found");
      writer.Bool(true);
      return { it->second };
    }

  private:
    std::function<T()> d_keyGetter;
    std::map<T,std::shared_ptr<Option>> d_optionByKey;
  };
}
