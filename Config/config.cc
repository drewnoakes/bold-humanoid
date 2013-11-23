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
    cerr << ccolor::error << "[Config::initialise] Parse error in file " << metadataFile << ": " << metaDocument.GetParseError() << ccolor::reset << endl;
    throw runtime_error("Parse error in configuration metadata JSON.");
  }

  if (confDocument.HasParseError())
  {
    cerr << ccolor::error << "[Config::initialise] Parse error in file " << configFile << ": " << confDocument.GetParseError() << ccolor::reset << endl;
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
      cerr << ccolor::error << "[Config::processLevel] 'type' property must have a string value" << ccolor::reset << endl;
      throw runtime_error("JSON 'type' property must have a string value");
    }

    bool isReadOnly = metaNode->TryGetBoolValue("readonly", false);
    bool isAdvanced = metaNode->TryGetBoolValue("advanced", false);

    auto type = string(typeMember->value.GetString());

    // TODO SETTINGS this mega-if-block should use some kind of polymorphism -- we'll need that later anyway

    if (type == "double")
    {
      double value;
      if (!metaNode->TryGetDoubleValue("default", &value))
      {
        cerr << ccolor::error << "[Config::processLevel] 'default' value for '" << path << "' must be a double" << ccolor::reset << endl;
        throw runtime_error("JSON 'default' value must be a double");
      }

      if (confNode)
      {
        if (!confNode->IsNumber())
        {
          cerr << ccolor::error << "[Config::processLevel] Configuration value for '" << path << "' must be a double" << ccolor::reset << endl;
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
        cerr << ccolor::error << "[Config::processLevel] 'default' value for '" << path << "' must be an int" << ccolor::reset << endl;
        throw runtime_error("JSON 'default' value must be an int");
      }

      if (confNode)
      {
        if (!confNode->IsInt())
        {
          cerr << ccolor::error << "[Config::processLevel] Configuration value for '" << path << "' must be an int" << ccolor::reset << endl;
          throw runtime_error("JSON configuration value must be an int");
        }
        value = confNode->GetInt();
      }

      auto min = metaNode->TryGetIntValue("min", numeric_limits<int>::min());
      auto max = metaNode->TryGetIntValue("max", numeric_limits<int>::max());
      Config::addSetting(new IntSetting(path, min, max, value, isReadOnly, isAdvanced));
    }
    else if (type == "enum")
    {
      int value;
      if (!metaNode->TryGetIntValue("default", &value))
      {
        cerr << ccolor::error << "[Config::processLevel] 'default' value for '" << path << "' must be an int" << ccolor::reset << endl;
        throw runtime_error("JSON 'default' value must be an int");
      }

      if (confNode)
      {
        if (!confNode->IsInt())
        {
          cerr << ccolor::error << "[Config::processLevel] Configuration value for '" << path << "' must be an int" << ccolor::reset << endl;
          throw runtime_error("JSON configuration value must be an int");
        }
        value = confNode->GetInt();
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

      Config::addSetting(new EnumSetting(path, pairs, value, isReadOnly, isAdvanced));
    }
    else if (type == "bool")
    {
      bool value;
      if (!metaNode->TryGetBoolValue("default", &value))
      {
        cerr << ccolor::error << "[Config::processLevel] 'default' value for '" << path << "' must be a bool" << ccolor::reset << endl;
        throw runtime_error("JSON 'default' value must be a bool");
      }

      if (confNode)
      {
        if (!confNode->IsBool())
        {
          cerr << ccolor::error << "[Config::processLevel] Configuration value for '" << path << "' must be a bool" << ccolor::reset << endl;
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
        cerr << ccolor::error << "[Config::processLevel] 'default' value for '" << path << "' must be a string" << ccolor::reset << endl;
        throw runtime_error("JSON 'default' value must be a string");
      }

      if (confNode)
      {
        if (!confNode->IsString())
        {
          cerr << ccolor::error << "[Config::processLevel] Configuration value for '" << path << "' must be a string" << ccolor::reset << endl;
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
          cerr << ccolor::error << "[Config::processLevel] Pixel label value for '" << path << "' must be an object" << ccolor::reset << endl;
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
          cerr << ccolor::error << "[Config::processLevel] Double range value for '" << path << "' must be a JSON array" << ccolor::reset << endl;
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

      auto confChildMember = confNode->FindMember(childName.c_str());
      Value* confChild = confChildMember ? &confChildMember->value : nullptr;

      if (!it->value.IsObject())
      {
        cerr << ccolor::warning << "[Config::processLevel] Skipping non-object: " << childName << ccolor::reset << endl;
        continue;
      }

      processLevel(&it->value, confChild, treeNode, newPath.str(), childName);
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
