#include "setting-implementations.hh"

using namespace bold;
using namespace std;

IntSetting::IntSetting(std::string path, int min, int max, int defaultValue, bool isReadOnly, bool isAdvanced)
: Setting(path, "int", isReadOnly, isAdvanced, defaultValue),
  d_min(min),
  d_max(max),
  d_defaultValue(defaultValue)
{}

bool IntSetting::isValidValue(int value) const
{
  return value >= d_min && value <= d_max;
}

void IntSetting::writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
{
  writer.Int(getValue());
}

void IntSetting::writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
{
  writer.String("default").Int(d_defaultValue);
  if (d_min != -std::numeric_limits<int>::max())
    writer.String("min").Int(d_min);
  if (d_max != std::numeric_limits<int>::max())
    writer.String("max").Int(d_max);
}

bool IntSetting::setValueFromJson(rapidjson::Value* value)
{
  if (!value->IsInt())
  {
    cerr << ccolor::error << "[IntSetting::setValueFromJson] Configuration value for '" << getPath() << "' must be an integer" << ccolor::reset << endl;
    return false;
  }

  return setValue(value->GetInt());
}

///////////////////////////////////////////////////////////

EnumSetting::EnumSetting(std::string path, std::map<int,std::string> pairs, int defaultValue, bool isReadOnly, bool isAdvanced)
: Setting(path, "enum", isReadOnly, isAdvanced, defaultValue),
  d_pairs(pairs),
  d_defaultValue(defaultValue)
{}

bool EnumSetting::isValidValue(int value) const
{
  return d_pairs.find(value) != d_pairs.end();
}

void EnumSetting::writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
{
  writer.Int(getValue());
}

void EnumSetting::writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
{
  writer.String("default").Int(d_defaultValue);
  writer.String("values");
  writer.StartArray();
  {
    for (auto const& pair : d_pairs) {
      writer.StartObject().String(pair.second.c_str()).Int(pair.first).EndObject();
    }
  }
  writer.EndArray();
}

bool EnumSetting::setValueFromJson(rapidjson::Value* value)
{
  if (!value->IsInt())
  {
    cerr << ccolor::error << "[EnumSetting::setValueFromJson] Configuration value for '" << getPath() << "' must be an integer" << ccolor::reset << endl;
    return false;
  }

  return setValue(value->GetInt());
}

///////////////////////////////////////////////////////////

DoubleSetting::DoubleSetting(std::string path, double min, double max, double defaultValue, bool isReadOnly, bool isAdvanced)
: Setting(path, "double", isReadOnly, isAdvanced, defaultValue),
  d_min(min),
  d_max(max),
  d_defaultValue(defaultValue)
{}

bool DoubleSetting::isValidValue(double value) const
{
  return value >= d_min && value <= d_max;
}

void DoubleSetting::writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
{
  writer.Double(getValue());
}

void DoubleSetting::writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
{
  writer.String("default").Double(d_defaultValue);
  if (d_min != -std::numeric_limits<double>::max())
    writer.String("min").Double(d_min);
  if (d_max != std::numeric_limits<double>::max())
    writer.String("max").Double(d_max);
}

bool DoubleSetting::setValueFromJson(rapidjson::Value* value)
{
  if (!value->IsNumber())
  {
    cerr << ccolor::error << "[DoubleSetting::setValueFromJson] Configuration value for '" << getPath() << "' must be a double" << ccolor::reset << endl;
    return false;
  }

  return setValue(value->GetDouble());
}

///////////////////////////////////////////////////////////

BoolSetting::BoolSetting(std::string path, bool defaultValue, bool isReadOnly, bool isAdvanced)
: Setting(path, "bool", isReadOnly, isAdvanced, defaultValue),
  d_defaultValue(defaultValue)
{}

bool BoolSetting::isValidValue(bool value) const
{
  return true;
}

void BoolSetting::writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
{
  writer.Bool(getValue());
}

void BoolSetting::writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
{
  writer.String("default").Bool(d_defaultValue);
}

bool BoolSetting::setValueFromJson(rapidjson::Value* value)
{
  if (!value->IsBool())
  {
    cerr << ccolor::error << "[BoolSetting::setValueFromJson] Configuration value for '" << getPath() << "' must be a bool" << ccolor::reset << endl;
    return false;
  }

  return setValue(value->GetBool());
}

///////////////////////////////////////////////////////////

HsvRangeSetting::HsvRangeSetting(std::string path, Colour::hsvRange defaultValue, bool isReadOnly, bool isAdvanced)
: Setting(path, "hsv-range", isReadOnly, isAdvanced, defaultValue),
  d_defaultValue(defaultValue)
{}

void HsvRangeSetting::writeHsvRangeJsonObject(rapidjson::Writer<rapidjson::StringBuffer>& writer, Colour::hsvRange const& value)
{
  writer.StartObject();
  {
    writer.String("hue").StartArray().Double(value.hMin).Double(value.hMax).EndArray();
    writer.String("sat").StartArray().Double(value.sMin).Double(value.sMax).EndArray();
    writer.String("val").StartArray().Double(value.vMin).Double(value.vMax).EndArray();
  }
  writer.EndObject();
}

bool HsvRangeSetting::tryParseJsonValue(rapidjson::Value* value, Colour::hsvRange* hsvRange)
{
  if (!value->IsObject())
  {
    cerr << ccolor::error << "[HsvRangeSetting::tryParseObject] JSON value must be an object" << ccolor::reset << endl;
    return false;
  }

  // {"hue":[44,60],"sat":[158,236],"val":[124,222]}

  auto parseChannel = [value](string channel, uchar* min, uchar* max)
  {
    auto mem = value->FindMember(channel.c_str());

    if (!mem || !mem->value.IsArray() || mem->value.Size() != 2 || !mem->value[0u].IsInt() || !mem->value[1u].IsInt())
    {
      cerr << ccolor::error << "[HsvRangeSetting::tryParseObject] hsv-range value for '" << channel << "' must be an array of two integer values" << ccolor::reset << endl;
      return false;
    }

    auto v1 = mem->value[0u].GetInt();
    auto v2 = mem->value[1u].GetInt();

    if (v1 < 0 || v1 > 255 || v2 < 0 || v2 > 255)
    {
      cerr << ccolor::error << "[HsvRangeSetting::tryParseObject] hsv-range value for '" << channel << "' must be an array of integers between 0 and 255, inclusive" << ccolor::reset << endl;
      return false;
    }

    *min = (uchar)v1,
    *max = (uchar)v2;
    return true;
  };

  if (!parseChannel("hue", &hsvRange->hMin, &hsvRange->hMax) ||
      !parseChannel("sat", &hsvRange->sMin, &hsvRange->sMax) ||
      !parseChannel("val", &hsvRange->vMin, &hsvRange->vMax))
    return false;

  if (!hsvRange->isValid())
  {
    cerr << ccolor::error << "[HsvRangeSetting::tryParseObject] hsv-range value parsed correctly but has invalid data" << ccolor::reset << endl;
    return false;
  }

  return true;
}

bool HsvRangeSetting::isValidValue(Colour::hsvRange value) const
{
  return value.isValid();
}

void HsvRangeSetting::writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
{
  writeHsvRangeJsonObject(writer, getValue());
}

void HsvRangeSetting::writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
{
  writer.String("default");
  writeHsvRangeJsonObject(writer, d_defaultValue);
}

bool HsvRangeSetting::setValueFromJson(rapidjson::Value* value)
{
  Colour::hsvRange hsvRange;

  if (!tryParseJsonValue(value, &hsvRange))
    return false;

  setValue(hsvRange);
  return true;
}

///////////////////////////////////////////////////////////

DoubleRangeSetting::DoubleRangeSetting(std::string path, Range<double> defaultValue, bool isReadOnly, bool isAdvanced)
: Setting(path, "double-range", isReadOnly, isAdvanced, defaultValue),
  d_defaultValue(defaultValue)
{}

void DoubleRangeSetting::writeDoubleRangeJsonObject(rapidjson::Writer<rapidjson::StringBuffer>& writer, Range<double> const& value)
{
  writer.StartArray().Double(value.min()).Double(value.max()).EndArray();
}

bool DoubleRangeSetting::tryParseJsonValue(rapidjson::Value* value, Range<double>* range)
{
  if (!value->IsArray() || value->Size() != 2 || !(*value)[0u].IsNumber() || !(*value)[1u].IsNumber())
  {
    cerr << ccolor::error << "[DoubleRangeSetting::tryParseJsonValue] Double range value must be a JSON array of two double values" << ccolor::reset << endl;
    return false;
  }

  auto v1 = (*value)[0u].GetDouble();
  auto v2 = (*value)[1u].GetDouble();

  if (v1 > v2)
  {
    cerr << ccolor::error << "[DoubleRangeSetting::tryParseJsonValue] Double range must have min <= max" << ccolor::reset << endl;
    return false;
  }

  *range = Range<double>(v1, v2);
  return true;
}

bool DoubleRangeSetting::isValidValue(Range<double> value) const
{
  return !value.isEmpty() && value.min() <= value.max();
}

void DoubleRangeSetting::writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
{
  writeDoubleRangeJsonObject(writer, getValue());
}

void DoubleRangeSetting::writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
{
  writer.String("default");
  writeDoubleRangeJsonObject(writer, d_defaultValue);
}

bool DoubleRangeSetting::setValueFromJson(rapidjson::Value* value)
{
  Range<double> range;

  if (!tryParseJsonValue(value, &range))
    return false;

  setValue(range);
  return true;
}

///////////////////////////////////////////////////////////

StringSetting::StringSetting(std::string path, std::string defaultValue, bool isReadOnly, bool isAdvanced)
: Setting(path, "string", isReadOnly, isAdvanced, defaultValue),
  d_defaultValue(defaultValue)
{}

bool StringSetting::isValidValue(std::string value) const
{
  return value.size() > 0;
}

void StringSetting::writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
{
  writer.String(getValue().c_str());
}

void StringSetting::writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
{
  writer.String("default").String(d_defaultValue.c_str());
}

bool StringSetting::setValueFromJson(rapidjson::Value* value)
{
  if (!value->IsString())
  {
    cerr << ccolor::error << "[StringSetting::setValueFromJson] Configuration value for '" << getPath() << "' must be a string" << ccolor::reset << endl;
    return false;
  }

  return setValue(value->GetString());
}
