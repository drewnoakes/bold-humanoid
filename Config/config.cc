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
    cerr << ccolor::error << "[Config::initialise] File not found: " << metadataFile << ccolor::reset << endl;
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
      Config::addSetting(new DoubleSetting(path, min, max, defaultValue, isReadOnly, isAdvanced));
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
      Config::addSetting(new IntSetting(path, min, max, defaultValue, isReadOnly, isAdvanced));
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

      Config::addSetting(new EnumSetting(path, pairs, defaultValue, isReadOnly, isAdvanced));
    }
    else if (type == "bool")
    {
      bool defaultValue;
      if (!metaNode->TryGetBoolValue("default", &defaultValue))
      {
        cerr << ccolor::error << "[Config::processLevel] 'default' value for '" << path << "' must be a bool" << ccolor::reset << endl;
        throw runtime_error("JSON 'default' value must be a bool");
      }

      Config::addSetting(new BoolSetting(path, defaultValue, isReadOnly, isAdvanced));
    }
    else if (type == "string")
    {
      const char* defaultValue;
      if (!metaNode->TryGetStringValue("default", &defaultValue))
      {
        cerr << ccolor::error << "[Config::processLevel] 'default' value for '" << path << "' must be a string" << ccolor::reset << endl;
        throw runtime_error("JSON 'default' value must be a string");
      }

      Config::addSetting(new StringSetting(path, defaultValue, isReadOnly, isAdvanced));
    }
    else if (type == "hsv-range")
    {
      auto defaultMember = metaNode->FindMember("default");

      Colour::hsvRange defaultValue;

      if (defaultMember && !HsvRangeSetting::tryParseJsonValue(&defaultMember->value, &defaultValue))
        throw runtime_error("Unable to parse hsv-range");

      Config::addSetting(new HsvRangeSetting(path, defaultValue, isReadOnly, isAdvanced));
    }
    else if (type == "double-range")
    {
      auto defaultMember = metaNode->FindMember("default");

      Range<double> defaultValue;

      if (defaultMember && !DoubleRangeSetting::tryParseJsonValue(&defaultMember->value, &defaultValue))
        throw runtime_error("Unable to parse double-range");

      Config::addSetting(new DoubleRangeSetting(path, defaultValue, isReadOnly, isAdvanced));
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
  rapidjson::Value const* configValue = d_configDocument;
  while (true)
  {
    size_t end = path.find(delimiter, start);

    auto nodeName = end != string::npos
      ? path.substr(start, end - start)
      : path.substr(start);

    auto member = configValue->FindMember(nodeName.c_str());

    if (!member)
      return nullptr;

    configValue = &member->value;

    if (!configValue)
      return nullptr;

    if (end == string::npos)
      return configValue;

    start = end + delimiter.length();
  }
}
