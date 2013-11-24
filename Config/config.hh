#pragma once

#include <iostream>
#include <functional>
#include <map>
#include <stack>
#include <rapidjson/document.h>

#include "../Setting/setting.hh"
#include "../util/ccolor.hh"

namespace bold
{
  class Action
  {
  public:
    Action(std::string id, std::string label, std::function<void()> callback)
    : d_id(id),
      d_label(label),
      d_callback(callback)
    {}

    /** Handles a request against this control. */
    void handleRequest() const { d_callback(); }

    std::string getId() const { return d_id; }
    std::string getLabel() const { return d_label; }

  private:
    std::string d_id;
    std::string d_label;
    std::function<void()> d_callback;
  };

  /// Central store for all configuration data.
  class Config
  {
  public:
    /// Retrieves a Setting<T> having the specified path.
    template<typename T>
    static Setting<T>* getSetting(std::string path)
    {
      std::string delimiter = ".";
      size_t start = 0;
      size_t end;
      TreeNode const* node = &d_root;
      while ((end = path.find(delimiter, start)) != std::string::npos)
      {
        auto nodeName = path.substr(start, end - start);
        start = end + delimiter.length();

        auto it = node->subNodeByName.find(nodeName);
        if (it == node->subNodeByName.end())
        {
          std::cerr << ccolor::warning << "[Config::getSetting] Requested setting with path '" << path << "' but no node was found with name: " << nodeName << ccolor::reset << std::endl;
          return nullptr;
        }
        node = &it->second;
      }

      auto settingName = path.substr(start);

      auto it = node->settingByName.find(settingName);

      if (it == node->settingByName.end())
      {
        std::cerr << ccolor::warning << "[Config::getSetting] Requested setting with path '" << path << "' but no setting was found with name: " << settingName << ccolor::reset << std::endl;
        return nullptr;
      }

      auto setting = it->second;

      // TODO check typeid

      return (Setting<T>*)setting;
    }

    /// Gets the current value of the specified setting. Throws if setting unknown.
    template<typename T>
    static T getValue(std::string path)
    {
      auto setting = getSetting<T>(path);
      if (!setting)
      {
        std::cerr << ccolor::error << "[Config::getValue] No setting exists with path: " << path << ccolor::reset << std::endl;
        throw std::runtime_error("No setting exists for requested path");
      }
      return setting->getValue();
    }

    /// Gets the current value of the specified setting. Throws if setting unknown or setting not readonly.
    template<typename T>
    static T getStaticValue(std::string path)
    {
      auto setting = getSetting<T>(path);
      if (!setting)
      {
        std::cerr << ccolor::error << "[Config::getValue] No setting exists with path: " << path << ccolor::reset << std::endl;
        throw std::runtime_error("No setting exists for requested path");
      }
      if (!setting->isReadOnly())
      {
        std::cerr << ccolor::error << "[Config::getValue] Requested static config value, however setting is not readonly: " << path << ccolor::reset << std::endl;
        throw std::runtime_error("Requested static config value, however setting is not readonly");
      }
      return setting->getValue();
    }

    /// Adds the specified Setting<T>.
    /// If a corresponding value exists in the config document, its value is
    /// retrieved and set on the provided setting.
    template<typename T>
    static void addSetting(Setting<T>* setting)
    {
      std::string path = setting->getPath();
      std::string delimiter = ".";
      size_t start = 0;
      size_t end;
      TreeNode* node = &d_root;
      rapidjson::Value* configValue = d_configDocument;
      while ((end = path.find(delimiter, start)) != std::string::npos)
      {
        auto nodeName = path.substr(start, end - start);
        start = end + delimiter.length();

        // Insert or create new tree node.
        auto ret = node->subNodeByName.insert(std::pair<std::string,TreeNode>(nodeName, TreeNode()));
        node = &ret.first->second;

        // Dereference the config node
        if (configValue)
        {
          auto member = configValue->FindMember(nodeName.c_str());
          configValue = member ? &member->value : nullptr;
        }
      }

      auto settingName = path.substr(start);

      // TODO SETTINGS validate that the setting name is not also used for a tree node

      node->settingByName[settingName] = setting;

      if (configValue != nullptr)
      {
        auto member = configValue->FindMember(settingName.c_str());
        if (member)
          setting->setValueFromJson(&member->value);
      }
    }

    static rapidjson::Value const* getConfigJsonValue(std::string path);

    static void initialise(std::string metadataFile, std::string configFile);

    static void initialisationCompleted() { assert(d_isInitialising); d_isInitialising = false; }
    static bool isInitialising() { return d_isInitialising; }

    static void addAction(std::string id, std::string label, std::function<void()> callback);

    static std::vector<Action*> getAllActions()
    {
      std::vector<Action*> actions;
      for (auto const& pair : d_actionById)
        actions.push_back(pair.second);
      return actions;
    }

    static Action* getAction(std::string id)
    {
      auto it = d_actionById.find(id);
      if (it == d_actionById.end())
        return nullptr;
      return it->second;
    }

    static std::vector<SettingBase*> getAllSettings()
    {
      std::vector<SettingBase*> settings;

      std::stack<TreeNode const*> stack;
      stack.push(&d_root);
      while (!stack.empty())
      {
        TreeNode const* node = stack.top();
        stack.pop();
        for (auto const& pair : node->settingByName)
          settings.push_back(pair.second);
        for (auto const& pair : node->subNodeByName)
          stack.push(&pair.second);
      }
      return settings;
    }

  private:
    struct TreeNode
    {
      TreeNode()
      : settingByName(),
        subNodeByName()
      {}

      std::map<std::string,SettingBase*> settingByName;
      std::map<std::string,TreeNode> subNodeByName;
    };

    static void processConfigMetaJsonValue(rapidjson::Value* metaNode, TreeNode* treeNode, std::string path, std::string name);

    Config() {}

    static TreeNode d_root;
    static std::map<std::string,Action*> d_actionById;
    static rapidjson::Document* d_configDocument;
    static bool d_isInitialising;
  };
}
