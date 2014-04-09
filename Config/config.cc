#include "config.hh"

#include <sstream>
#include <limits>
#include <memory>

#include "../PixelLabel/pixellabel.hh"
#include "../Setting/setting-implementations.hh"
#include "../util/Range.hh"
#include "../util/log.hh"

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

using namespace bold;
using namespace rapidjson;
using namespace std;

Config::TreeNode Config::d_root;
map<string,Action*> Config::d_actionById;
vector<unique_ptr<Document const>> Config::d_configDocuments;
vector<string> Config::d_configFileNames;
bool Config::d_isInitialising = true;
sigc::signal<void, SettingBase*> Config::updated;

unique_ptr<Document const> loadJsonDocument(std::string path)
{
  FILE* file = fopen(path.c_str(), "rb");

  if (!file)
  {
    log::error("loadJsonDocument") << "File not found: " << path;
    throw runtime_error("Configuration metadata file not found.");
  }

  char buffer[65536];

  Document* doc = new Document();
  FileReadStream stream(file, buffer, sizeof(buffer));
  doc->ParseStream<0, UTF8<>, FileReadStream>(stream);

  if (fclose(file))
    log::error("loadJsonDocument") << "Error closing file: " << strerror(errno) << " (" << errno << ")";

  if (doc->HasParseError())
  {
    log::error("loadJsonDocument") << "Parse error in file " << path << ": " << doc->GetParseError();
    delete doc;
    throw runtime_error("Parse error in JSON file");
  }

  return unique_ptr<Document const>(doc);
}

void Config::initialise(string metadataFile, string configFile)
{
  if (d_configDocuments.size() != 0 || !d_isInitialising)
  {
    log::error("Config::initialise") << "Already initialised";
    throw runtime_error("Configuration already initialised.");
  }

  // Walk through the metadata, creating settings as found.
  //
  // For each setting, attempt to set the initial value from the config file,
  // otherwise fall back to the default value specified in the metadata.

  log::info("Config::initialise") << "Parsing configuration metadata";

  // Load the single metadata JSON file
  auto metaDocument = loadJsonDocument(metadataFile);

  // Load the configuration document cascade
  string path = configFile;
  while (true)
  {
    // The config document
    log::info("Config::initialise") << "Parsing configuration file: " << path;
    auto confDocument = loadJsonDocument(path);

    // Check whether a parent is specified in the 'inherits' property
    auto it = confDocument->FindMember("inherits");
    auto hasParent = it && it->value.IsString();
    if (hasParent)
      path = it->value.GetString();

    d_configDocuments.push_back(move(confDocument));
    d_configFileNames.push_back(path);

    // Terminate the loop if we have no more parents to process
    if (!hasParent)
      break;
  }

  // Walk the metadata document, building up a tree of Setting<T> objects
  processConfigMetaJsonValue(metaDocument.get(), &d_root, "", "");

  // TODO defaults are a bit confusing -- remove them from the meta document, and change this to "Reload config from disk"
  addAction("config.reset-defaults", "Reset default config", []
  {
    for (SettingBase* setting : getAllSettings())
    {
      if (!setting->isReadOnly())
        setting->resetToDefaultValue();
    }
  });
}

SettingBase* Config::getSettingBase(string path)
{
  string delimiter = ".";
  size_t start = 0;
  size_t end;
  TreeNode const* node = &d_root;
  while ((end = path.find(delimiter, start)) != string::npos)
  {
    auto nodeName = path.substr(start, end - start);
    start = end + delimiter.length();

    auto it = node->subNodeByName.find(nodeName);
    if (it == node->subNodeByName.end())
    {
      if (d_root.subNodeByName.size() == 0 && d_root.settingByName.size() == 0)
        log::warning("Config::getSettingBase") << "Requested setting with path '" << path << "' but not settings have been loaded yet.";
      else
        log::warning("Config::getSettingBase") << "Requested setting with path '" << path << "' but no node was found with name: " << nodeName;
      return nullptr;
    }
    node = &it->second;
  }

  auto settingName = path.substr(start);

  auto it = node->settingByName.find(settingName);

  if (it == node->settingByName.end())
  {
    log::warning("Config::getSettingBase") << "Requested setting with path '" << path << "' but no setting was found with name: " << settingName;
    return nullptr;
  }

  return it->second;
}

void Config::processConfigMetaJsonValue(Value const* metaNode, TreeNode* treeNode, string path, string name)
{
  assert(metaNode->IsObject());

  // Any JSON object that defines a 'type' member is considered a value definition
  auto typeMember = metaNode->FindMember("type");

  if (typeMember)
  {
    // process setting

    if (!typeMember->value.IsString())
    {
      log::error("Config::processLevel") << "'type' property must have a string value";
      throw runtime_error("JSON 'type' property must have a string value");
    }

    bool isReadOnly = metaNode->TryGetBoolValue("readonly", false);
    const char* descriptionChars = metaNode->TryGetStringValue("description", (const char*)nullptr);
    string description = descriptionChars == nullptr ? "" : descriptionChars;

    auto type = string(typeMember->value.GetString());

    // TODO SETTINGS replace mega-if-block with lookup in map<string,function<bool(Value*)>> populated with static factories on *Setting classes

    if (type == "double")
    {
      double defaultValue;
      if (!metaNode->TryGetDoubleValue("default", &defaultValue))
      {
        log::error("Config::processLevel") << "'default' value for '" << path << "' must be a double";
        throw runtime_error("JSON 'default' value must be a double");
      }

      auto min = metaNode->TryGetDoubleValue("min", -numeric_limits<double>::max());
      auto max = metaNode->TryGetDoubleValue("max", numeric_limits<double>::max());
      Config::addSetting(new DoubleSetting(path, min, max, defaultValue, isReadOnly, description));
    }
    else if (type == "int")
    {
      int defaultValue;
      if (!metaNode->TryGetIntValue("default", &defaultValue))
      {
        log::error("Config::processLevel") << "'default' value for '" << path << "' must be an int";
        throw runtime_error("JSON 'default' value must be an int");
      }

      auto min = metaNode->TryGetIntValue("min", -numeric_limits<int>::max());
      auto max = metaNode->TryGetIntValue("max", numeric_limits<int>::max());
      Config::addSetting(new IntSetting(path, min, max, defaultValue, isReadOnly, description));
    }
    else if (type == "enum")
    {
      int defaultValue;
      if (!metaNode->TryGetIntValue("default", &defaultValue))
      {
        log::error("Config::processLevel") << "'default' value for '" << path << "' must be an int";
        throw runtime_error("JSON 'default' value must be an int");
      }

      auto valuesObj = metaNode->FindMember("values");
      if (!valuesObj || !valuesObj->value.IsObject())
      {
        log::error("Config::processLevel") << "Configuration value for enum '" << path << "' must specify values";
        throw runtime_error("JSON configuration for enum must specify values");
      }

      map<int, string> pairs;
      for (auto it = valuesObj->value.MemberBegin(); it != valuesObj->value.MemberEnd(); it++)
      {
        if (!it->value.IsInt())
        {
          log::error("Config::processLevel") << "Configuration value for enum '" << path << "' has member with non-integral value";
          throw runtime_error("JSON configuration for enum must have integral values");
        }
        pairs[it->value.GetInt()] = it->name.GetString();
      }

      Config::addSetting(new EnumSetting(path, pairs, defaultValue, isReadOnly, description));
    }
    else if (type == "bool")
    {
      bool defaultValue;
      if (!metaNode->TryGetBoolValue("default", &defaultValue))
      {
        log::error("Config::processLevel") << "'default' value for '" << path << "' must be a bool";
        throw runtime_error("JSON 'default' value must be a bool");
      }

      Config::addSetting(new BoolSetting(path, defaultValue, isReadOnly, description));
    }
    else if (type == "string")
    {
      const char* defaultValue;
      if (!metaNode->TryGetStringValue("default", &defaultValue))
      {
        log::error("Config::processLevel") << "'default' value for '" << path << "' must be a string";
        throw runtime_error("JSON 'default' value must be a string");
      }

      Config::addSetting(new StringSetting(path, defaultValue, isReadOnly, description));
    }
    else if (type == "string[]")
    {
      auto defaultMember = metaNode->FindMember("default");

      vector<string> defaultValue;

      if (defaultMember && !StringArraySetting::tryParseJsonValue(&defaultMember->value, &defaultValue))
        throw runtime_error("Unable to parse string[]");

      Config::addSetting(new StringArraySetting(path, defaultValue, isReadOnly, description));
    }
    else if (type == "hsv-range")
    {
      auto defaultMember = metaNode->FindMember("default");

      Colour::hsvRange defaultValue;

      if (defaultMember && !HsvRangeSetting::tryParseJsonValue(&defaultMember->value, &defaultValue))
        throw runtime_error("Unable to parse hsv-range");

      Config::addSetting(new HsvRangeSetting(path, defaultValue, isReadOnly, description));
    }
    else if (type == "double-range")
    {
      auto defaultMember = metaNode->FindMember("default");

      Range<double> defaultValue;

      if (defaultMember && !DoubleRangeSetting::tryParseJsonValue(&defaultMember->value, &defaultValue))
        throw runtime_error("Unable to parse double-range");

      Config::addSetting(new DoubleRangeSetting(path, defaultValue, isReadOnly, description));
    }
    else if (type == "bgr-colour")
    {
      auto defaultMember = metaNode->FindMember("default");

      Colour::bgr defaultValue;

      if (defaultMember && !BgrColourSetting::tryParseJsonValue(&defaultMember->value, &defaultValue))
        throw runtime_error("Unable to parse bgr-colour");

      Config::addSetting(new BgrColourSetting(path, defaultValue, isReadOnly, description));
    }
    else
    {
      log::error("Config::processLevel") << "Unsupported 'type' property value: " << type;
      throw runtime_error("Unsupported JSON 'type' property value");
    }

    return;
  }

  // Recurse through child objects of this node

  for (auto it = metaNode->MemberBegin(); it != metaNode->MemberEnd(); it++)
  {
    if (it->value.IsObject())
    {
      string childName = it->name.GetString();
      stringstream newPath;
      if (path.size())
        newPath << path << ".";
      newPath << childName;

      if (!it->value.IsObject())
      {
        log::warning("Config::processLevel") << "Skipping non-object: " << childName;
        continue;
      }

      processConfigMetaJsonValue(&it->value, treeNode, newPath.str(), childName);
    }
  }
}

void Config::addSetting(SettingBase* setting)
{
  string path = setting->getPath();
  string delimiter = ".";
  size_t start = 0;
  size_t end;
  TreeNode* node = &d_root;
  while ((end = path.find(delimiter, start)) != string::npos)
  {
    auto nodeName = path.substr(start, end - start);
    start = end + delimiter.length();

    // Insert or create new tree node.
    auto ret = node->subNodeByName.insert(pair<string,TreeNode>(nodeName, TreeNode()));
    node = &ret.first->second;
  }

  auto settingName = path.substr(start);
  Value const* configValue = Config::getConfigJsonValue(path);

  // Validate that the setting name is not also used for a tree node
  if (node->subNodeByName.find(settingName) != node->subNodeByName.end())
  {
    log::error("Config::addSetting") << "Attempt to add setting but node already exists with path: " << setting->getPath();
    throw runtime_error("Attempt to add setting over existing node");
  }

  // Insert, ensuring a setting does not already exist with that name
  if (!node->settingByName.insert(make_pair(settingName, setting)).second)
  {
    log::error("Config::addSetting") << "Attempt to add duplicate setting with path: " << setting->getPath();
    throw runtime_error("Attempt to add duplicate setting");
  }

  // If a config value exists for this setting, set it
  if (configValue != nullptr)
    setting->setValueFromJson(configValue);

  // Propagate change events globally
  setting->changedBase.connect([](SettingBase* s){ Config::updated.emit(s); });
}

void Config::addAction(string const& id, string const& label,
                       function<void()> callback)
{
  Action* action = new Action(id, label, callback);
  assert(!action->hasArguments());
  auto it = d_actionById.insert(make_pair(id, action));

  if (it.second == false)
  {
    delete action;
    log::error("Config::addAction") << "Action with id '" << id << "' already registered";
    throw runtime_error("Action already registered with provided id");
  }
}

void Config::addAction(string const& id, string const& label,
                       function<void(rapidjson::Value*)> callback)
{
  Action* action = new Action(id, label, callback);
  assert(action->hasArguments());
  auto it = d_actionById.insert(make_pair(id, action));

  if (it.second == false)
  {
    delete action;
    log::error("Config::addAction") << "Action with id '" << id << "' already registered";
    throw runtime_error("Action already registered with provided id");
  }
}

Value const* Config::getConfigJsonValue(string path)
{
  if (d_configDocuments.size() == 0)
  {
    log::error("Config::getConfigJsonValue") << "No config documents have been set";
    throw runtime_error("No config documents have been set");
  }

  for (unique_ptr<Document const> const& configValue : d_configDocuments)
  {
    // Walk down the tree of each config document, looking for a match to 'path'
    // TODO the config files could be flatted into a single map, reducing the complexity of lookups by some small amount
    string delimiter = ".";
    size_t start = 0;
    Value const* node = configValue.get();

    while (true)
    {
      size_t end = path.find(delimiter, start);

      auto childName = end != string::npos
        ? path.substr(start, end - start)
        : path.substr(start);

      auto childMember = node->FindMember(childName.c_str());

      if (!childMember)
      {
        // No match was found in this file, so break to the outer
        // loop to look in the next document.
        break;
      }

      node = &childMember->value;

      assert(node);

      if (end == string::npos)
        return node;

      // Continue along the string, evaluating the next level down in the tree
      start = end + delimiter.length();
    }
  }

  // No match was made in any of the config files
  return nullptr;
}

vector<Action*> Config::getAllActions()
{
  vector<Action*> actions;
  for (auto const& pair : d_actionById)
    actions.push_back(pair.second);
  return actions;
}

Action* Config::getAction(string id)
{
  auto it = d_actionById.find(id);
  if (it == d_actionById.end())
    return nullptr;
  return it->second;
}

vector<SettingBase*> Config::getAllSettings()
{
  vector<SettingBase*> settings;

  stack<TreeNode const*> stack;
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

vector<SettingBase*> Config::getSettings(string prefix)
{
  string delimiter = ".";
  size_t start = 0;
  TreeNode const* node = &d_root;
  while (true)
  {
    size_t end = prefix.find(delimiter, start);

    auto nodeName = end != string::npos
      ? prefix.substr(start, end - start)
      : prefix.substr(start);

    auto child = node->subNodeByName.find(nodeName);

    if (child == node->subNodeByName.end())
      return vector<SettingBase*>();

    node = &child->second;

    if (end == string::npos)
    {
      // Return all the settings in the current node
      vector<SettingBase*> settings;
      for (auto pair : node->settingByName)
        settings.push_back(pair.second);
      return settings;
    }

    start = end + delimiter.length();
  }
}
