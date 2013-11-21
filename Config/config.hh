#pragma once

#include <iostream>
#include <functional>
#include <map>
#include <rapidjson/document.h>

#include "../Setting/setting.hh"

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
          std::cerr << "[Config::getSetting] Requested setting with path '" << path << "' but no node was found with name: " << nodeName << std::endl;
          return nullptr;
        }
        node = &it->second;
      }

      auto settingName = path.substr(start);

      auto it = node->settingByName.find(settingName);

      if (it == node->settingByName.end())
      {
        std::cerr << "[Config::getSetting] Requested setting with path '" << path << "' but no setting was found with name: " << settingName << std::endl;
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
        std::cerr << "[Config::getValue] No setting exists with path: " << path << std::endl;
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
        std::cerr << "[Config::getValue] No setting exists with path: " << path << std::endl;
        throw std::runtime_error("No setting exists for requested path");
      }
      if (!setting->isReadOnly())
      {
        std::cerr << "[Config::getValue] Requested static config value, however setting is not readonly: " << path << std::endl;
        throw std::runtime_error("Requested static config value, however setting is not readonly");
      }
      return setting->getValue();
    }

    /// Adds the specified Setting<T>.
    template<typename T>
    static void addSetting(Setting<T>* setting)
    {
      std::string path = setting->getPath();
      std::string delimiter = ".";
      size_t start = 0;
      size_t end;
      TreeNode* node = &d_root;
      while ((end = path.find(delimiter, start)) != std::string::npos)
      {
        auto nodeName = path.substr(start, end - start);
        start = end + delimiter.length();

        // Insert or create new tree node.
        auto ret = node->subNodeByName.insert(std::pair<std::string,TreeNode>(nodeName, TreeNode()));
        node = &ret.first->second;
      }

      auto settingName = path.substr(start);

      // TODO validate that the setting name is not also used for a tree node

      node->settingByName[settingName] = setting;
    }

    static void initialise(std::string metadataFile, std::string configFile);

    static void addAction(std::string id, std::string label, std::function<void()> callback);

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

    static void processLevel(rapidjson::Value* metaNode, rapidjson::Value* confNode, TreeNode* treeNode, std::string path, std::string name);

    Config() {}

    static TreeNode d_root;
    static std::map<std::string,Action*> d_actionById;
  };
}
