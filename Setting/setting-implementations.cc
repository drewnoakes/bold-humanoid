#include "setting-implementations.hh"

#include "../util/log.hh"
#include "../Config/config.hh"

using namespace bold;
using namespace rapidjson;
using namespace std;

IntSetting::IntSetting(string path, int min, int max, bool isReadOnly, string description)
: Setting(path, "int", isReadOnly, description),
  d_min(min),
  d_max(max)
{}

bool IntSetting::isValidValue(int const& value) const
{
  return value >= d_min && value <= d_max;
}

string IntSetting::getValidationMessage(int const& value) const
{
  if (value < d_min || value > d_max)
  {
    stringstream msg;
    msg << "Value must be between " << d_min << " and " << d_max;
    return msg.str();
  }
  return "";
}

bool IntSetting::tryParseJsonValue(Value const* jsonValue, int* parsedValue) const
{
  if (jsonValue == nullptr || !jsonValue->IsInt())
    return false;
  *parsedValue = jsonValue->GetInt();
  return true;
}

void IntSetting::writeJsonValue(Writer<StringBuffer>& writer, int const& value) const
{
  writer.Int(value);
}

void IntSetting::writeJsonMetadata(Writer<StringBuffer>& writer) const
{
  Setting<int>::writeJsonMetadata(writer);

  if (d_min != -numeric_limits<int>::max())
  {
    writer.String("min");
    writer.Int(d_min);
  }

  if (d_max != numeric_limits<int>::max())
  {
    writer.String("max");
    writer.Int(d_max);
  }
}

///////////////////////////////////////////////////////////

EnumSetting::EnumSetting(string path, map<int,string> pairs, bool isReadOnly, string description)
: Setting(path, "enum", isReadOnly, description),
  d_pairs(pairs)
{}

bool EnumSetting::isValidValue(int const& value) const
{
  return d_pairs.find(value) != d_pairs.end();
}

string EnumSetting::getValidationMessage(int const& value) const
{
  if (d_pairs.find(value) == d_pairs.end())
  {
    stringstream msg;
    msg << "Value " << value << " does not exist in the enumeration";
    return msg.str();
  }
  return "";
}

bool EnumSetting::tryParseJsonValue(Value const* jsonValue, int* parsedValue) const
{
  if (jsonValue == nullptr || !jsonValue->IsInt())
    return false;
  *parsedValue = jsonValue->GetInt();
  return true;
}

void EnumSetting::writeJsonValue(Writer<StringBuffer>& writer, int const& value) const
{
  writer.Int(value);
}

void EnumSetting::writeJsonMetadata(Writer<StringBuffer>& writer) const
{
  Setting<int>::writeJsonMetadata(writer);

  writer.String("values");
  writer.StartArray();
  {
    for (auto const& pair : d_pairs)
    {
      writer.StartObject();
      writer.String("text");
      writer.String(pair.second.c_str());
      writer.String("value");
      writer.Int(pair.first);
      writer.EndObject();
    }
  }
  writer.EndArray();
}

///////////////////////////////////////////////////////////

DoubleSetting::DoubleSetting(string path, double min, double max, bool isReadOnly, string description)
: Setting(path, "double", isReadOnly, description),
  d_min(min),
  d_max(max)
{}

bool DoubleSetting::isValidValue(double const& value) const
{
  return value >= d_min && value <= d_max;
}

string DoubleSetting::getValidationMessage(double const& value) const
{
  if (value < d_min || value > d_max)
  {
    stringstream msg;
    msg << "Value must be between " << d_min << " and " << d_max;
    return msg.str();
  }
  return "";
}

bool DoubleSetting::tryParseJsonValue(Value const* jsonValue, double* parsedValue) const
{
  if (jsonValue == nullptr || !jsonValue->IsNumber())
    return false;
  *parsedValue = jsonValue->GetDouble();
  return true;
}

void DoubleSetting::writeJsonValue(Writer<StringBuffer>& writer, double const& value) const
{
  writer.Double(value);
}

void DoubleSetting::writeJsonMetadata(Writer<StringBuffer>& writer) const
{
  Setting<double>::writeJsonMetadata(writer);

  if (d_min != -numeric_limits<double>::max())
  {
    writer.String("min");
    writer.Double(d_min);
  }
  if (d_max != numeric_limits<double>::max())
  {
    writer.String("max");
    writer.Double(d_max);
  }
}

///////////////////////////////////////////////////////////

BoolSetting::BoolSetting(string path, bool isReadOnly, string description)
: Setting(path, "bool", isReadOnly, description)
{}

bool BoolSetting::tryParseJsonValue(Value const* jsonValue, bool* parsedValue) const
{
  if (jsonValue == nullptr || !jsonValue->IsBool())
    return false;
  *parsedValue = jsonValue->GetBool();
  return true;
}

void BoolSetting::writeJsonValue(Writer<StringBuffer>& writer, bool const& value) const
{
  writer.Bool(value);
}

///////////////////////////////////////////////////////////

HsvRangeSetting::HsvRangeSetting(string path, bool isReadOnly, string description)
: Setting(path, "hsv-range", isReadOnly, description)
{}

bool HsvRangeSetting::isValidValue(Colour::hsvRange const& value) const
{
  return value.isValid();
}

string HsvRangeSetting::getValidationMessage(Colour::hsvRange const& value) const
{
  if (!value.isValid())
    return "Sat/Val max values must be greater than min values";
  return "";
}

bool HsvRangeSetting::tryParseJsonValue(Value const* jsonValue, Colour::hsvRange* parsedValue) const
{
  if (!jsonValue->IsObject())
  {
    log::error("HsvRangeSetting::tryParseObject") << "JSON value must be an object";
    return false;
  }

  // {"hue":[44,60],"sat":[158,236],"val":[124,222]}

  auto parseChannel = [ jsonValue ](string channel, uchar* min, uchar* max)
  {
    auto mem = jsonValue->FindMember(channel.c_str());

    if (mem == jsonValue->MemberEnd() || !mem->value.IsArray() || mem->value.Size() != 2 || !mem->value[0u].IsInt() || !mem->value[1u].IsInt())
    {
      log::error("HsvRangeSetting::tryParseObject") << "hsv-range value for '" << channel << "' must be an array of two integer values";
      return false;
    }

    auto v1 = mem->value[0u].GetInt();
    auto v2 = mem->value[1u].GetInt();

    if (v1 < 0 || v1 > 255 || v2 < 0 || v2 > 255)
    {
      log::error("HsvRangeSetting::tryParseObject") << "hsv-range value for '" << channel << "' must be an array of integers between 0 and 255, inclusive";
      return false;
    }

    *min = (uchar)v1,
      *max = (uchar)v2;
    return true;
  };

  if (!parseChannel("hue", &parsedValue->hMin, &parsedValue->hMax) ||
    !parseChannel("sat", &parsedValue->sMin, &parsedValue->sMax) ||
    !parseChannel("val", &parsedValue->vMin, &parsedValue->vMax))
    return false;

  if (!parsedValue->isValid())
  {
    log::error("HsvRangeSetting::tryParseObject") << "hsv-range value parsed correctly but has invalid data";
    return false;
  }

  return true;
}

void HsvRangeSetting::writeJsonValue(Writer<StringBuffer>& writer, Colour::hsvRange const& value) const
{
  writer.StartObject();
  {
    writer.String("hue");
    writer.StartArray();
    writer.Double(value.hMin);
    writer.Double(value.hMax);
    writer.EndArray();

    writer.String("sat");
    writer.StartArray();
    writer.Double(value.sMin);
    writer.Double(value.sMax);
    writer.EndArray();

    writer.String("val");
    writer.StartArray();
    writer.Double(value.vMin);
    writer.Double(value.vMax);
    writer.EndArray();
  }
  writer.EndObject();
}

///////////////////////////////////////////////////////////

DoubleRangeSetting::DoubleRangeSetting(string path, bool isReadOnly, string description)
: Setting(path, "double-range", isReadOnly, description)
{}

bool DoubleRangeSetting::isValidValue(Range<double> const& value) const
{
  return !value.isEmpty() && value.min() <= value.max();
}

string DoubleRangeSetting::getValidationMessage(Range<double> const& value) const
{
  if (value.isEmpty())
    return "Range may not be empty";
  if (value.min() > value.max())
    return "Range's min may not be greater than its max";
  return "";
}

bool DoubleRangeSetting::tryParseJsonValue(Value const* jsonValue, Range<double>* parsedValue) const
{
  if (!jsonValue->IsArray() || jsonValue->Size() != 2 || !(*jsonValue)[0u].IsNumber() || !(*jsonValue)[1u].IsNumber())
  {
    log::error("DoubleRangeSetting::tryParseJsonValue") << "Double range value must be a JSON array of two double values";
    return false;
  }

  auto v1 = (*jsonValue)[0u].GetDouble();
  auto v2 = (*jsonValue)[1u].GetDouble();

  if (v1 > v2)
  {
    log::error("DoubleRangeSetting::tryParseJsonValue") << "Double range must have min <= max";
    return false;
  }

  *parsedValue = Range<double>(v1, v2);
  return true;
}

void DoubleRangeSetting::writeJsonValue(Writer<StringBuffer>& writer, Range<double> const& value) const
{
  writer.StartArray();
  writer.Double(value.min());
  writer.Double(value.max());
  writer.EndArray();
}

///////////////////////////////////////////////////////////

StringSetting::StringSetting(string path, bool isReadOnly, string description)
: Setting(path, "string", isReadOnly, description)
{}

bool StringSetting::isValidValue(string const& value) const
{
  return value.size() > 0;
}

string StringSetting::getValidationMessage(string const& value) const
{
  if (value.size() == 0)
    return "String may not have zero length";
  return "";
}

bool StringSetting::tryParseJsonValue(Value const* jsonValue, string* parsedValue) const
{
  if (jsonValue == nullptr || !jsonValue->IsString())
    return false;
  *parsedValue = jsonValue->GetString();
  return true;
}

void StringSetting::writeJsonValue(Writer<StringBuffer>& writer, string const& value) const
{
  writer.String(value.c_str());
}

///////////////////////////////////////////////////////////

StringArraySetting::StringArraySetting(string path, bool isReadOnly, string description)
: Setting(path, "string[]", isReadOnly, description)
{}

bool StringArraySetting::areValuesEqual(vector<string> const& a, vector<string> const& b) const
{
  if (a.size() != b.size())
    return false;
  for (uint i = 0; i < a.size(); i++)
    if (a[i] != b[i])
      return false;
  return true;
}

bool StringArraySetting::tryParseJsonValue(Value const* jsonValue, vector<string>* parsedValue) const
{
  if (jsonValue == nullptr)
    return false;

  if (!jsonValue->IsArray())
    return false;

  *parsedValue = vector<string>();

  for (uint i = 0; i < jsonValue->Size(); i++)
  {
    Value const& v = (*jsonValue)[i];
    if (!v.IsString())
      return false;
    parsedValue->push_back(v.GetString());
  }

  return true;
}

void StringArraySetting::writeJsonValue(Writer<StringBuffer>& writer, vector<string> const& value) const
{
  writer.StartArray();
  {
    for (auto const& s : value)
      writer.String(s.c_str());
  }
  writer.EndArray();
}

///////////////////////////////////////////////////////////

BgrColourSetting::BgrColourSetting(string path, bool isReadOnly, string description)
  : Setting(path, "bgr-colour", isReadOnly, description)
{}

bool BgrColourSetting::tryParseJsonValue(Value const* jsonValue, Colour::bgr* parsedValue) const
{
  if (jsonValue == nullptr)
    return false;

  if (!jsonValue->IsObject())
    return false;

  auto bMember = jsonValue->FindMember("b");
  auto gMember = jsonValue->FindMember("g");
  auto rMember = jsonValue->FindMember("r");

  if (bMember == jsonValue->MemberEnd()
    || gMember == jsonValue->MemberEnd()
    || rMember == jsonValue->MemberEnd())
    return false;

  if (!bMember->value.IsInt() || !gMember->value.IsInt() || !rMember->value.IsInt())
    return false;

  *parsedValue = Colour::bgr(bMember->value.GetInt(), gMember->value.GetInt(), rMember->value.GetInt());
  return true;
}

void BgrColourSetting::writeJsonValue(Writer<StringBuffer>& writer, const Colour::bgr& value) const
{
  writer.StartObject();
  {
    writer.String("r");
    writer.Int(value.r);
    writer.String("g");
    writer.Int(value.g);
    writer.String("b");
    writer.Int(value.b);
  }
  writer.EndObject();
}
