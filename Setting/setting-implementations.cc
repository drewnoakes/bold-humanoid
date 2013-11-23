#include "setting-implementations.hh"

using namespace bold;

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
  if (d_min != std::numeric_limits<int>::min())
    writer.String("min").Int(d_min);
  if (d_max != std::numeric_limits<int>::max())
    writer.String("max").Int(d_max);
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
  if (d_min != std::numeric_limits<double>::min())
    writer.String("min").Double(d_min);
  if (d_max != std::numeric_limits<double>::max())
    writer.String("max").Double(d_max);
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

///////////////////////////////////////////////////////////

HsvRangeSetting::HsvRangeSetting(std::string path, Colour::hsvRange defaultValue, bool isReadOnly, bool isAdvanced)
: Setting(path, "hsv-range", isReadOnly, isAdvanced, defaultValue),
  d_defaultValue(defaultValue)
{}

bool HsvRangeSetting::isValidValue(Colour::hsvRange value) const
{
  return value.isValid();
}

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

void HsvRangeSetting::writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
{
  writeHsvRangeJsonObject(writer, getValue());
}

void HsvRangeSetting::writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
{
  writer.String("default");
  writeHsvRangeJsonObject(writer, d_defaultValue);
}

///////////////////////////////////////////////////////////

DoubleRangeSetting::DoubleRangeSetting(std::string path, Range<double> defaultValue, bool isReadOnly, bool isAdvanced)
: Setting(path, "double-range", isReadOnly, isAdvanced, defaultValue),
  d_defaultValue(defaultValue)
{}

bool DoubleRangeSetting::isValidValue(Range<double> value) const
{
  return !value.isEmpty() && value.min() <= value.max();
}

void DoubleRangeSetting::writeDoubleRangeJsonObject(rapidjson::Writer<rapidjson::StringBuffer>& writer, Range<double> const& value)
{
  writer.StartArray().Double(value.min()).Double(value.max()).EndArray();
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
