#pragma once

#include <iostream>
#include <functional>
#include <map>
#include <rapidjson/document.h>
#include <sigc++/signal.h>
#include <stack>

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
    static sigc::signal<void, SettingBase*> updated;

    static SettingBase* getSettingBase(std::string path);

    /// Retrieves a Setting<T> having the specified path.
    template<typename T>
    static Setting<T>* getSetting(std::string path)
    {
      auto setting = getSettingBase(path);

      // TODO SETTINGS check typeid

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

    /// Adds the specified setting.
    /// If a corresponding value exists in the config document, its value is
    /// retrieved and set on the provided setting.
    static void addSetting(SettingBase* setting);

    static void addAction(std::string id, std::string label, std::function<void()> callback);

    static rapidjson::Value const* getConfigJsonValue(std::string path);

    static void initialise(std::string metadataFile, std::string configFile);

    static void initialisationCompleted() { assert(d_isInitialising); d_isInitialising = false; }
    static bool isInitialising() { return d_isInitialising; }

    static Action* getAction(std::string id);
    static std::vector<Action*> getAllActions();
    static std::vector<SettingBase*> getSettings(std::string prefix);
    static std::vector<SettingBase*> getAllSettings();

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
