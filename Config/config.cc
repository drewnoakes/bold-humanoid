#include "config.hh"

#include <sstream>
#include <limits>

#include "../PixelLabel/pixellabel.hh"
#include "../util/Range.hh"

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

using namespace bold;
using namespace rapidjson;
using namespace std;

Config::TreeNode Config::d_root;
map<string,Action*> Config::d_actionById;

void Config::initialise(string metadataFile, string configFile)
{
  // Walk through the metadata, creating settings as found.
  //
  // For each setting, attempt to set the initial value from the config file,
  // otherwise fall back to the default value specified in the metadata.

  cout << "[Config::initialise] Parsing configuration" << endl;

  FILE* mf = fopen(metadataFile.c_str(), "rb");
  FILE* cf = fopen(configFile.c_str(), "rb");

  char buffer[65536];

  Document metaDocument;
  FileReadStream metaStream(mf, buffer, sizeof(buffer));
  metaDocument.ParseStream<0, UTF8<>, FileReadStream>(metaStream);

  Document confDocument;
  FileReadStream confStream(cf, buffer, sizeof(buffer));
  confDocument.ParseStream<0, UTF8<>, FileReadStream>(confStream);

  if (metaDocument.HasParseError())
  {
    cerr << "[Config::initialise] Parse error in file " << metadataFile << ": " << metaDocument.GetParseError() << endl;
    throw runtime_error("Parse error in configuration metadata JSON.");
  }

  if (confDocument.HasParseError())
  {
    cerr << "[Config::initialise] Parse error in file " << configFile << ": " << confDocument.GetParseError() << endl;
    throw runtime_error("Parse error in configuration JSON.");
  }

  processLevel(&metaDocument, &confDocument, &d_root, "", "");
}

void Config::processLevel(Value* metaNode, Value* confNode, TreeNode* treeNode, string path, string name)
{
  assert(metaNode->IsObject());

  // Any JSON object that defines a 'type' member is considered a value definition
  auto typeMember = metaNode->FindMember("type");

  if (typeMember)
  {
    // process setting

    if (!typeMember->value.IsString())
    {
      cerr << "[Config::processLevel] 'type' property must have a string value" << endl;
      throw runtime_error("JSON 'type' property must have a string value");
    }

    bool isReadOnly = metaNode->TryGetBoolValue("readonly", false);
    bool isAdvanced = metaNode->TryGetBoolValue("advanced", false);

    auto type = string(typeMember->value.GetString());

    // TODO this mega-if-block should use some kind of polymorphism -- we'll need that later anyway'

    if (type == "double")
    {
      double value;
      if (!metaNode->TryGetDoubleValue("default", &value))
      {
        cerr << "[Config::processLevel] 'default' value for '" << path << "' must be a double" << endl;
        throw runtime_error("JSON 'default' value must be a double");
      }

      if (confNode)
      {
        if (!confNode->IsNumber())
        {
          cerr << "[Config::processLevel] Configuration value for '" << path << "' must be a double" << endl;
          throw runtime_error("JSON configuration value must be a double");
        }
        value = confNode->GetDouble();
      }

      auto min = metaNode->TryGetDoubleValue("min", numeric_limits<double>::min());
      auto max = metaNode->TryGetDoubleValue("max", numeric_limits<double>::max());
      Config::addSetting(new DoubleSetting(path, min, max, value, isReadOnly, isAdvanced));
    }
    else if (type == "int")
    {
      int value;
      if (!metaNode->TryGetIntValue("default", &value))
      {
        cerr << "[Config::processLevel] 'default' value for '" << path << "' must be an int" << endl;
        throw runtime_error("JSON 'default' value must be an int");
      }

      if (confNode)
      {
        if (!confNode->IsInt())
        {
          cerr << "[Config::processLevel] Configuration value for '" << path << "' must be an int" << endl;
          throw runtime_error("JSON configuration value must be an int");
        }
        value = confNode->GetInt();
      }

      auto min = metaNode->TryGetIntValue("min", numeric_limits<int>::min());
      auto max = metaNode->TryGetIntValue("max", numeric_limits<int>::max());
      Config::addSetting(new IntSetting(path, min, max, value, isReadOnly, isAdvanced));
    }
    else if (type == "bool")
    {
      bool value;
      if (!metaNode->TryGetBoolValue("default", &value))
      {
        cerr << "[Config::processLevel] 'default' value for '" << path << "' must be a bool" << endl;
        throw runtime_error("JSON 'default' value must be a bool");
      }

      if (confNode)
      {
        if (!confNode->IsBool())
        {
          cerr << "[Config::processLevel] Configuration value for '" << path << "' must be a bool" << endl;
          throw runtime_error("JSON configuration value must be a bool");
        }
        value = confNode->GetBool();
      }

      Config::addSetting(new BoolSetting(path, value, isReadOnly, isAdvanced));
    }
    else if (type == "string")
    {
      const char* value;
      if (!metaNode->TryGetStringValue("default", &value))
      {
        cerr << "[Config::processLevel] 'default' value for '" << path << "' must be a string" << endl;
        throw runtime_error("JSON 'default' value must be a string");
      }

      if (confNode)
      {
        if (!confNode->IsString())
        {
          cerr << "[Config::processLevel] Configuration value for '" << path << "' must be a string" << endl;
          throw runtime_error("JSON configuration value must be a string");
        }
        value = confNode->GetString();
      }

      Config::addSetting(new StringSetting(path, value, isReadOnly, isAdvanced));
    }
    else if (type == "pixel-label")
    {
      auto parseObject = [name,path](Value* value) {
        if (!value->IsObject())
        {
          cerr << "[Config::processLevel] Pixel label value for '" << path << "' must be an object" << endl;
          throw runtime_error("JSON pixel label value must be an object");
        }

        Colour::hsvRange hsvRange;

        // TODO populate HSV range

        return PixelLabel(hsvRange, name);
      };

      auto defaultMember = metaNode->FindMember("default");

      PixelLabel value;

      if (defaultMember)
        value = parseObject(&defaultMember->value);

      if (confNode)
        value = parseObject(confNode);

      Config::addSetting(new PixelLabelSetting(path, value, isReadOnly, isAdvanced));
    }
    else if (type == "double-range")
    {
      auto parseObject = [path](Value* value) {
        if (!value->IsArray())
        {
          cerr << "[Config::processLevel] Double range value for '" << path << "' must be a JSON array" << endl;
          throw runtime_error("JSON double range value must be an object");
        }

        // TODO populate double range

        return Range<double>(0, 1);
      };

      auto defaultMember = metaNode->FindMember("default");

      Range<double> value;

      if (defaultMember)
        value = parseObject(&defaultMember->value);

      if (confNode)
        value = parseObject(confNode);

      Config::addSetting(new DoubleRangeSetting(path, value, isReadOnly, isAdvanced));
    }
    else
    {
      cerr << "[Config::processLevel] Unsupported 'type' property value: " << type << endl;
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

      auto confChildMember = confNode->FindMember(childName.c_str());
      Value* confChild = confChildMember ? &confChildMember->value : nullptr;

      if (!it->value.IsObject())
      {
        cerr << "[Config::processLevel] Skipping non-object: " << childName << endl;
        continue;
      }

      processLevel(&it->value, confChild, treeNode, newPath.str(), childName);
    }
  }
}

void Config::addAction(string id, string label, function<void()> callback)
{
  auto it = d_actionById.insert(pair<string,function<void()>(id, callback));

  if (it.second == false)
  {
    cerr << "[Config::addAction] Action with id '" << id << "' already registered" << endl;
    throw runtime_error("Action already registered with provided id");
  }
}
