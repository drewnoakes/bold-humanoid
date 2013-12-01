#include "config.hh"

#include <sstream>
#include <limits>

#include "../PixelLabel/pixellabel.hh"
#include "../Setting/setting-implementations.hh"
#include "../util/Range.hh"

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

using namespace bold;
using namespace rapidjson;
using namespace std;

Config::TreeNode Config::d_root;
map<string,Action*> Config::d_actionById;
Document* Config::d_configDocument;
bool Config::d_isInitialising = true;
sigc::signal<void, SettingBase*> Config::updated;

void Config::initialise(string metadataFile, string configFile)
{
  if (d_configDocument != nullptr || !d_isInitialising)
  {
    cerr << ccolor::error << "[Config::initialise] Already initialised" << ccolor::reset << endl;
    throw runtime_error("Configuration already initialised.");
  }

  // Walk through the metadata, creating settings as found.
  //
  // For each setting, attempt to set the initial value from the config file,
  // otherwise fall back to the default value specified in the metadata.

  cout << "[Config::initialise] Parsing configuration" << endl;

  FILE* mf = fopen(metadataFile.c_str(), "rb");
  FILE* cf = fopen(configFile.c_str(), "rb");

  if (!mf)
  {
    cerr << ccolor::error << "[Config::initialise] File not found: " << metadataFile << ccolor::reset << endl;
    throw runtime_error("Configuration metadata file not found.");
  }

  if (!cf)
  {
    cerr << ccolor::error << "[Config::initialise] File not found: " << configFile << ccolor::reset << endl;
    throw runtime_error("Configuration file not found.");
  }

  char buffer[65536];

  Document metaDocument;
  FileReadStream metaStream(mf, buffer, sizeof(buffer));
  metaDocument.ParseStream<0, UTF8<>, FileReadStream>(metaStream);

  Document* confDocument = new Document();
  FileReadStream confStream(cf, buffer, sizeof(buffer));
  confDocument->ParseStream<0, UTF8<>, FileReadStream>(confStream);

  if (metaDocument.HasParseError())
  {
    cerr << ccolor::error << "[Config::initialise] Parse error in file " << metadataFile << ": " << metaDocument.GetParseError() << ccolor::reset << endl;
    throw runtime_error("Parse error in configuration metadata JSON.");
  }

  if (confDocument->HasParseError())
  {
    cerr << ccolor::error << "[Config::initialise] Parse error in file " << configFile << ": " << confDocument->GetParseError() << ccolor::reset << endl;
    delete confDocument;
    throw runtime_error("Parse error in configuration JSON.");
  }

  d_configDocument = confDocument;

  processConfigMetaJsonValue(&metaDocument, &d_root, "", "");

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
      cerr << ccolor::warning << "[Config::getSettingBase] Requested setting with path '" << path << "' but no node was found with name: " << nodeName << ccolor::reset << endl;
      return nullptr;
    }
    node = &it->second;
  }

  auto settingName = path.substr(start);

  auto it = node->settingByName.find(settingName);

  if (it == node->settingByName.end())
  {
    cerr << ccolor::warning << "[Config::getSettingBase] Requested setting with path '" << path << "' but no setting was found with name: " << settingName << ccolor::reset << endl;
    return nullptr;
  }

  return it->second;
}

void Config::processConfigMetaJsonValue(Value* metaNode, TreeNode* treeNode, string path, string name)
{
  assert(metaNode->IsObject());

  // Any JSON object that defines a 'type' member is considered a value definition
  auto typeMember = metaNode->FindMember("type");

  if (typeMember)
  {
    // process setting

    if (!typeMember->value.IsString())
    {
      cerr << ccolor::error << "[Config::processLevel] 'type' property must have a string value" << ccolor::reset << endl;
      throw runtime_error("JSON 'type' property must have a string value");
    }

    bool isReadOnly = metaNode->TryGetBoolValue("readonly", false);
    bool isAdvanced = metaNode->TryGetBoolValue("advanced", false);
    const char* descriptionChars = metaNode->TryGetStringValue("description", (const char*)nullptr);
    string description = descriptionChars == nullptr ? "" : descriptionChars;

    auto type = string(typeMember->value.GetString());

    // TODO SETTINGS replace mega-if-block with lookup in map<string,function<bool(Value*)>> populated with static factories on *Setting classes

    if (type == "double")
    {
      double defaultValue;
      if (!metaNode->TryGetDoubleValue("default", &defaultValue))
      {
        cerr << ccolor::error << "[Config::processLevel] 'default' value for '" << path << "' must be a double" << ccolor::reset << endl;
        throw runtime_error("JSON 'default' value must be a double");
      }

      auto min = metaNode->TryGetDoubleValue("min", -numeric_limits<double>::max());
      auto max = metaNode->TryGetDoubleValue("max", numeric_limits<double>::max());
      Config::addSetting(new DoubleSetting(path, min, max, defaultValue, isReadOnly, isAdvanced, description));
    }
    else if (type == "int")
    {
      int defaultValue;
      if (!metaNode->TryGetIntValue("default", &defaultValue))
      {
        cerr << ccolor::error << "[Config::processLevel] 'default' value for '" << path << "' must be an int" << ccolor::reset << endl;
        throw runtime_error("JSON 'default' value must be an int");
      }

      auto min = metaNode->TryGetIntValue("min", -numeric_limits<int>::max());
      auto max = metaNode->TryGetIntValue("max", numeric_limits<int>::max());
      Config::addSetting(new IntSetting(path, min, max, defaultValue, isReadOnly, isAdvanced, description));
    }
    else if (type == "enum")
    {
      int defaultValue;
      if (!metaNode->TryGetIntValue("default", &defaultValue))
      {
        cerr << ccolor::error << "[Config::processLevel] 'default' value for '" << path << "' must be an int" << ccolor::reset << endl;
        throw runtime_error("JSON 'default' value must be an int");
      }

      auto valuesObj = metaNode->FindMember("values");
      if (!valuesObj || !valuesObj->value.IsObject())
      {
        cerr << ccolor::error << "[Config::processLevel] Configuration value for enum '" << path << "' must specify values" << ccolor::reset << endl;
        throw runtime_error("JSON configuration for enum must specify values");
      }

      map<int, string> pairs;
      for (auto it = valuesObj->value.MemberBegin(); it != valuesObj->value.MemberEnd(); it++)
      {
        if (!it->value.IsInt())
        {
          cerr << ccolor::error << "[Config::processLevel] Configuration value for enum '" << path << "' has member with non-integral value" << ccolor::reset << endl;
          throw runtime_error("JSON configuration for enum must have integral values");
        }
        pairs[it->value.GetInt()] = it->name.GetString();
      }

      Config::addSetting(new EnumSetting(path, pairs, defaultValue, isReadOnly, isAdvanced, description));
    }
    else if (type == "bool")
    {
      bool defaultValue;
      if (!metaNode->TryGetBoolValue("default", &defaultValue))
      {
        cerr << ccolor::error << "[Config::processLevel] 'default' value for '" << path << "' must be a bool" << ccolor::reset << endl;
        throw runtime_error("JSON 'default' value must be a bool");
      }

      Config::addSetting(new BoolSetting(path, defaultValue, isReadOnly, isAdvanced, description));
    }
    else if (type == "string")
    {
      const char* defaultValue;
      if (!metaNode->TryGetStringValue("default", &defaultValue))
      {
        cerr << ccolor::error << "[Config::processLevel] 'default' value for '" << path << "' must be a string" << ccolor::reset << endl;
        throw runtime_error("JSON 'default' value must be a string");
      }

      Config::addSetting(new StringSetting(path, defaultValue, isReadOnly, isAdvanced, description));
    }
    else if (type == "hsv-range")
    {
      auto defaultMember = metaNode->FindMember("default");

      Colour::hsvRange defaultValue;

      if (defaultMember && !HsvRangeSetting::tryParseJsonValue(&defaultMember->value, &defaultValue))
        throw runtime_error("Unable to parse hsv-range");

      Config::addSetting(new HsvRangeSetting(path, defaultValue, isReadOnly, isAdvanced, description));
    }
    else if (type == "double-range")
    {
      auto defaultMember = metaNode->FindMember("default");

      Range<double> defaultValue;

      if (defaultMember && !DoubleRangeSetting::tryParseJsonValue(&defaultMember->value, &defaultValue))
        throw runtime_error("Unable to parse double-range");

      Config::addSetting(new DoubleRangeSetting(path, defaultValue, isReadOnly, isAdvanced, description));
    }
    else if (type == "bgr-colour")
    {
      auto defaultMember = metaNode->FindMember("default");

      Colour::bgr defaultValue;

      if (defaultMember && !BgrColourSetting::tryParseJsonValue(&defaultMember->value, &defaultValue))
        throw runtime_error("Unable to parse bgr-colour");

      Config::addSetting(new BgrColourSetting(path, defaultValue, isReadOnly, isAdvanced, description));
    }
    else
    {
      cerr << ccolor::error << "[Config::processLevel] Unsupported 'type' property value: " << type << ccolor::reset << endl;
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
        cerr << ccolor::warning << "[Config::processLevel] Skipping non-object: " << childName << ccolor::reset << endl;
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
  Value* configValue = d_configDocument;
  while ((end = path.find(delimiter, start)) != string::npos)
  {
    auto nodeName = path.substr(start, end - start);
    start = end + delimiter.length();

    // Insert or create new tree node.
    auto ret = node->subNodeByName.insert(pair<string,TreeNode>(nodeName, TreeNode()));
    node = &ret.first->second;

    // Dereference the config node
    if (configValue)
    {
      auto member = configValue->FindMember(nodeName.c_str());
      configValue = member ? &member->value : nullptr;
    }
  }

  auto settingName = path.substr(start);

  // Validate that the setting name is not also used for a tree node
  if (node->subNodeByName.find(settingName) != node->subNodeByName.end())
  {
    cerr << ccolor::error << "[Config::addSetting] Attempt to add setting but node already exists with path: " << setting->getPath() << ccolor::reset << endl;
    throw runtime_error("Attempt to add setting over existing node");
  }

  // Insert, ensuring a setting does not already exist with that name
  if (!node->settingByName.insert(make_pair(settingName, setting)).second)
  {
    cerr << ccolor::error << "[Config::addSetting] Attempt to add duplicate setting with path: " << setting->getPath() << ccolor::reset << endl;
    throw runtime_error("Attempt to add duplicate setting");
  }

  // If a config value exists for this setting, set it
  if (configValue != nullptr)
  {
    auto member = configValue->FindMember(settingName.c_str());
    if (member)
      setting->setValueFromJson(&member->value);
  }

  setting->changedBase.connect([](SettingBase* s){ Config::updated.emit(s); });
}

void Config::addAction(string id, string label, function<void()> callback)
{
  Action* action = new Action(id, label, callback);
  auto it = d_actionById.insert(make_pair(id, action));

  if (it.second == false)
  {
    delete action;
    cerr << ccolor::error << "[Config::addAction] Action with id '" << id << "' already registered" << ccolor::reset << endl;
    throw runtime_error("Action already registered with provided id");
  }
}

Value const* Config::getConfigJsonValue(string path)
{
  string delimiter = ".";
  size_t start = 0;
  Value const* configValue = d_configDocument;

  if (!configValue)
  {
    cerr << ccolor::error << "[Config::getConfigJsonValue] Config document has not yet been set" << ccolor::reset << endl;
    throw runtime_error("Config document has not yet been set");
  }

  while (true)
  {
    size_t end = path.find(delimiter, start);

    auto nodeName = end != string::npos
      ? path.substr(start, end - start)
      : path.substr(start);

    assert(configValue);

    auto member = configValue->FindMember(nodeName.c_str());

    if (!member)
      return nullptr;

    configValue = &member->value;

    assert(configValue);

    if (end == string::npos)
      return configValue;

    start = end + delimiter.length();
  }
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
