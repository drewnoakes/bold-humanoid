#pragma once

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
      d_callback([callback](rapidjson::Value* doc) { callback(); }),
      d_requiresArguments(false)
    {}

    Action(std::string id, std::string label, std::function<void(rapidjson::Value*)> callback)
    : d_id(id),
      d_label(label),
      d_callback(callback),
      d_requiresArguments(true)
    {}

    /** Handles a request against this control. */
    void handleRequest() const { d_callback(nullptr); }
    /** Handles a request against this control that includes JSON arguments. */
    void handleRequest(rapidjson::Value* args) const { d_callback(args); }

    std::string getId() const { return d_id; }
    std::string getLabel() const { return d_label; }
    bool hasArguments() const { return d_requiresArguments; }

  private:
    std::string d_id;
    std::string d_label;
    std::function<void(rapidjson::Value*)> d_callback;
    bool d_requiresArguments;
  };

  /// Central store for all configuration data.
  class Config
  {
  public:
    static sigc::signal<void, SettingBase const*> updated;

    static SettingBase* getSettingBase(std::string path);

    /// Retrieves a Setting<T> having the specified path.
    template<typename T>
    static Setting<T>* getSetting(std::string path)
    {
      auto setting = getSettingBase(path);

      std::type_index typeIndex = std::is_enum<T>::value ? typeid(int) : typeid(T);

      if (typeIndex != setting->getTypeIndex())
      {
        log::error("Config::getSetting") << "Attempt to get setting having different type: " << path;
        throw std::runtime_error("Attempt to get setting having different type");
      }

      return (Setting<T>*)setting;
    }

    /// Gets the current value of the specified setting. Throws if setting unknown.
    template<typename T>
    static T getValue(std::string path)
    {
      auto setting = getSetting<T>(path);
      if (!setting)
      {
        log::error("Config::getValue") << "No setting exists with path: " << path;
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
        log::error("Config::getValue") << "No setting exists with path: " << path;
        throw std::runtime_error("No setting exists for requested path");
      }
      if (!setting->isReadOnly())
      {
        log::error("Config::getValue") << "Requested static config value, however setting is not readonly: " << path;
        throw std::runtime_error("Requested static config value, however setting is not readonly");
      }
      return setting->getValue();
    }

    /// Adds the specified setting.
    /// If a corresponding value exists in the config document, its value is
    /// retrieved and set on the provided setting.
    static void addSetting(SettingBase* setting);

    static void addAction(std::string const& id, std::string const& label, std::function<void()> callback);

    static void addAction(std::string const& id, std::string const& label, std::function<void(rapidjson::Value*)> callback);

    static rapidjson::Value const* getConfigJsonValue(std::string path);

    static void initialise(std::string metadataFile, std::string configFile);

    static void initialisationCompleted() { ASSERT(d_isInitialising); d_isInitialising = false; }
    static bool isInitialising() { return d_isInitialising; }

    static Action* getAction(std::string id);
    static std::vector<Action*> getAllActions();
    static std::vector<SettingBase*> getSettings(std::string prefix);
    static std::vector<SettingBase*> getAllSettings();

    static std::vector<std::string> getConfigDocumentNames() { return d_configFileNames; }

    static void setPermissible(bool permissible = true) { d_permissible = permissible; }

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

    static void processConfigMetaJsonValue(rapidjson::Value const* metaNode, TreeNode* treeNode, std::string path, std::string name);

    Config() = delete;

    static TreeNode d_root;
    static std::map<std::string,Action*> d_actionById;
    static std::vector<std::unique_ptr<rapidjson::Document const>> d_configDocuments;
    static std::vector<std::string> d_configFileNames;
    static bool d_isInitialising;
    static bool d_permissible;
  };
}
