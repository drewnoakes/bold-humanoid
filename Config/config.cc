#include "config.hh"

#include <sstream>
#include <limits>
#include <memory>

#include "../PixelLabel/pixellabel.hh"
#include "../Setting/setting-implementations.hh"
#include "../util/Range.hh"
#include "../util/log.hh"
#include "../util/json.hh"

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
sigc::signal<void, SettingBase const*> Config::updated;
bool Config::d_permissible = false;

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

void Config::initialise(string const& metadataFile, string const& configFile)
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
    auto hasParent = it != confDocument->MemberEnd() && it->value.IsString();
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

  addAction("config.reload-from-disk", "Reload config from disk", []
  {
    for (SettingBase* setting : getAllSettings())
    {
      if (!setting->isReadOnly())
        setting->resetToInitialValue();
    }
  });

  addAction("config.echo-changes", "Echo changes", []
  {
    // TODO build up a valid JSON configuration document

    for (SettingBase* setting : getAllSettings())
    {
      if (setting->isReadOnly() || !setting->isModified())
        continue;

      cout << "Setting " << setting->getPath() << " changed" << endl;
    }
  });
}

SettingBase* Config::getSettingBase(string const& path)
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

void Config::processConfigMetaJsonValue(Value const* metaNode, TreeNode* treeNode, string const& path, string const& name)
{
  ASSERT(metaNode->IsObject());

  // Any JSON object that defines a 'type' member is considered a value definition
  auto typeMember = metaNode->FindMember("type");

  if (typeMember != metaNode->MemberEnd())
  {
    // process setting

    if (!typeMember->value.IsString())
    {
      log::error("Config::processLevel") << "'type' property must have a string value";
      throw runtime_error("JSON 'type' property must have a string value");
    }

    bool isReadOnly = GetBoolWithDefault(*metaNode, "readonly", false);
    const char* descriptionChars = GetStringWithDefault(*metaNode, "description", (const char*)nullptr);
    string description = descriptionChars == nullptr ? "" : descriptionChars;

    auto type = string(typeMember->value.GetString());

    // TODO SETTINGS replace mega-if-block with lookup in map<string,function<bool(Value*)>> populated with static factories on *Setting classes

    SettingBase* setting;

    if (type == "double")
    {
      auto min = GetDoubleWithDefault(*metaNode, "min", -numeric_limits<double>::max());
      auto max = GetDoubleWithDefault(*metaNode, "max", numeric_limits<double>::max());
      setting = new DoubleSetting(path, min, max, isReadOnly, description);
    }
    else if (type == "int")
    {
      auto min = GetIntWithDefault(*metaNode, "min", -numeric_limits<int>::max());
      auto max = GetIntWithDefault(*metaNode, "max", numeric_limits<int>::max());
      setting = new IntSetting(path, min, max, isReadOnly, description);
    }
    else if (type == "enum")
    {
      auto valuesObj = metaNode->FindMember("values");
      if (valuesObj == metaNode->MemberEnd() || !valuesObj->value.IsObject())
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

      setting = new EnumSetting(path, pairs, isReadOnly, description);
    }
    else if (type == "bool")
    {
      setting = new BoolSetting(path, isReadOnly, description);
    }
    else if (type == "string")
    {
      setting = new StringSetting(path, isReadOnly, description);
    }
    else if (type == "string[]")
    {
      setting = new StringArraySetting(path, isReadOnly, description);
    }
    else if (type == "hsv-range")
    {
      setting = new HsvRangeSetting(path, isReadOnly, description);
    }
    else if (type == "double-range")
    {
      setting = new DoubleRangeSetting(path, isReadOnly, description);
    }
    else if (type == "bgr-colour")
    {
      setting = new BgrColourSetting(path, isReadOnly, description);
    }
    else
    {
      log::error("Config::processLevel") << "Unsupported 'type' property value: " << type;
      throw runtime_error("Unsupported JSON 'type' property value");
    }

    Value const* jsonValue = Config::getConfigJsonValue(path);
    setting->setValueFromJson(jsonValue);
    Config::addSetting(setting);
  }

  // Recur through child objects of this node

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

  // Find/create the node for this setting
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

  // Validate that the setting name is not also used for a tree node
  if (!d_permissible && node->subNodeByName.find(settingName) != node->subNodeByName.end())
  {
    log::error("Config::addSetting") << "Attempt to add setting but node already exists with path: " << setting->getPath();
    throw runtime_error("Attempt to add setting over existing node");
  }

  // Insert, ensuring a setting does not already exist with that name
  if (!d_permissible && !node->settingByName.insert(make_pair(settingName, setting)).second)
  {
    log::error("Config::addSetting") << "Attempt to add duplicate setting with path: " << setting->getPath();
    throw runtime_error("Attempt to add duplicate setting");
  }

  // Propagate change events globally
  setting->changedBase.connect([](SettingBase const* s) { Config::updated.emit(s); });
}

void Config::addAction(string const& id, string const& label,
                       function<void()> callback)
{
  Action* action = new Action(id, label, callback);
  ASSERT(!action->hasArguments());
  auto it = d_actionById.insert(make_pair(id, action));

  if (!d_permissible && it.second == false)
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
  ASSERT(action->hasArguments());
  auto it = d_actionById.insert(make_pair(id, action));

  if (!d_permissible && it.second == false)
  {
    delete action;
    log::error("Config::addAction") << "Action with id '" << id << "' already registered";
    throw runtime_error("Action already registered with provided id");
  }
}

Value const* Config::getConfigJsonValue(string const& path)
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

      if (childMember == node->MemberEnd())
      {
        // No match was found in this file, so break to the outer
        // loop to look in the next document.
        break;
      }

      node = &childMember->value;

      ASSERT(node);

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

Action* Config::getAction(string const& id)
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

vector<SettingBase*> Config::getSettings(string const& prefix)
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
