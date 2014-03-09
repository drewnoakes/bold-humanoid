#pragma once

#include "setting.hh"

#include <map>
#include <string>
#include <sigc++/signal.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <vector>

#include "../PixelLabel/pixellabel.hh"
#include "../util/Range.hh"
#include "../util/ccolor.hh"

namespace bold
{
  /// Models a setting with an integer value.
  class IntSetting : public Setting<int>
  {
  public:
    static bool tryParseJsonValue(rapidjson::Value const* value, int* i);

    IntSetting(std::string path, int min, int max, int defaultValue, bool isReadOnly, std::string description);
    ~IntSetting() {}

    int getMinimum() const { return d_min; }
    int getMaximum() const { return d_max; }
    bool isValidValue(int value) const override;
    std::string getValidationMessage(int value) const override;
    int getDefaultValue() const override { return d_defaultValue; }
    virtual bool setValueFromJson(rapidjson::Value const* value) override;
    virtual void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;
    virtual void writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    int d_min;
    int d_max;
    int d_defaultValue;
  };

  /// Models a setting with an integer value selected from a set of valid numbers.
  class EnumSetting : public Setting<int>
  {
  public:
    EnumSetting(std::string path, std::map<int,std::string> pairs, int defaultValue, bool isReadOnly, std::string description);
    ~EnumSetting() {}

    bool isValidValue(int value) const override;
    std::string getValidationMessage(int value) const override;
    int getDefaultValue() const override { return d_defaultValue; }
    virtual bool setValueFromJson(rapidjson::Value const* value) override;
    virtual void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;
    virtual void writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    std::map<int,std::string> d_pairs;
    int d_defaultValue;
  };

  /// Models a setting with a double value.
  class DoubleSetting : public Setting<double>
  {
  public:
    DoubleSetting(std::string path, double min, double max, double defaultValue, bool isReadOnly, std::string description);
    ~DoubleSetting() {}

    bool isValidValue(double value) const override;
    std::string getValidationMessage(double value) const override;
    double getDefaultValue() const override { return d_defaultValue; }
    virtual bool setValueFromJson(rapidjson::Value const* value) override;
    virtual void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;
    virtual void writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    double d_min;
    double d_max;
    double d_defaultValue;
  };

  /// Models a setting with a boolean value.
  class BoolSetting : public Setting<bool>
  {
  public:
    static bool tryParseJsonValue(rapidjson::Value const* value, bool* b);

    BoolSetting(std::string path, bool defaultValue, bool isReadOnly, std::string description);
    ~BoolSetting() {}

    bool isValidValue(bool value) const override;
    std::string getValidationMessage(bool value) const override;
    bool getDefaultValue() const override { return d_defaultValue; }
    virtual bool setValueFromJson(rapidjson::Value const* value) override;
    virtual void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;
    virtual void writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    bool d_defaultValue;
  };

  /// Models a setting with a Colour::hsvRange value.
  class HsvRangeSetting : public Setting<Colour::hsvRange>
  {
  public:
    static void writeHsvRangeJsonObject(rapidjson::Writer<rapidjson::StringBuffer>& writer, Colour::hsvRange const& value);
    static bool tryParseJsonValue(rapidjson::Value const* value, Colour::hsvRange* hsvRange);

    HsvRangeSetting(std::string path, Colour::hsvRange defaultValue, bool isReadOnly, std::string description);
    ~HsvRangeSetting() {}

    bool isValidValue(Colour::hsvRange value) const override;
    std::string getValidationMessage(Colour::hsvRange value) const override;
    Colour::hsvRange getDefaultValue() const override { return d_defaultValue; }
    virtual bool setValueFromJson(rapidjson::Value const* value) override;
    virtual void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;
    virtual void writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    Colour::hsvRange d_defaultValue;
  };

  /// Models a setting with a Range<double> value.
  class DoubleRangeSetting : public Setting<Range<double>>
  {
  public:
    static void writeDoubleRangeJsonObject(rapidjson::Writer<rapidjson::StringBuffer>& writer, Range<double> const& value);
    static bool tryParseJsonValue(rapidjson::Value const* value, Range<double>* hsvRange);

    DoubleRangeSetting(std::string path, Range<double> defaultValue, bool isReadOnly, std::string description);
    ~DoubleRangeSetting() {}

    bool isValidValue(Range<double> value) const override;
    std::string getValidationMessage(Range<double> value) const override;
    Range<double> getDefaultValue() const override { return d_defaultValue; }
    virtual bool setValueFromJson(rapidjson::Value const* value) override;
    virtual void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;
    virtual void writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    Range<double> d_defaultValue;
  };

  /// Models a setting with a std::string value.
  class StringSetting : public Setting<std::string>
  {
  public:
    StringSetting(std::string path, std::string defaultValue, bool isReadOnly, std::string description);
    ~StringSetting() {}

    bool isValidValue(std::string value) const override;
    std::string getValidationMessage(std::string value) const override;
    std::string getDefaultValue() const override { return d_defaultValue; }
    virtual bool setValueFromJson(rapidjson::Value const* value) override;
    virtual void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;
    virtual void writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    std::string d_defaultValue;
  };

  /// Models a setting with a std::string value.
  class StringArraySetting : public Setting<std::vector<std::string>>
  {
  public:
    static void writeStringArrayJsonObject(rapidjson::Writer<rapidjson::StringBuffer>& writer, std::vector<std::string> const& value);
    static bool tryParseJsonValue(rapidjson::Value const* value, std::vector<std::string>* strings);

    StringArraySetting(std::string path, std::vector<std::string> defaultValue, bool isReadOnly, std::string description);
    ~StringArraySetting() {}

    bool isValidValue(std::vector<std::string> value) const override;
    std::string getValidationMessage(std::vector<std::string> value) const override;
    std::vector<std::string> getDefaultValue() const override { return d_defaultValue; }
    virtual bool setValueFromJson(rapidjson::Value const* value) override;
    virtual void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;
    virtual void writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    std::vector<std::string> d_defaultValue;
  };

  /// Models a setting with a Colour::bgr value.
  class BgrColourSetting : public Setting<Colour::bgr>
  {
  public:
    static void writeBgrColourJsonObject(rapidjson::Writer<rapidjson::StringBuffer>& writer, Colour::bgr const& value);
    static bool tryParseJsonValue(rapidjson::Value const* value, Colour::bgr* bgr);

    BgrColourSetting(std::string path, Colour::bgr defaultValue, bool isReadOnly, std::string description);
    ~BgrColourSetting() {}

    bool isValidValue(Colour::bgr value) const override;
    std::string getValidationMessage(Colour::bgr value) const override;
    Colour::bgr getDefaultValue() const override { return d_defaultValue; }
    virtual bool setValueFromJson(rapidjson::Value const* value) override;
    virtual void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;
    virtual void writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    Colour::bgr d_defaultValue;
  };
}
