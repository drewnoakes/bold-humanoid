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
