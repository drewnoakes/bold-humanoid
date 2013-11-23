#pragma once

#include "setting.hh"

#include <iostream>
#include <map>
#include <string>
#include <sigc++/signal.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "../PixelLabel/pixellabel.hh"
#include "../util/Range.hh"
#include "../util/ccolor.hh"

namespace bold
{
  /// Models a setting with an integer value.
  class IntSetting : public Setting<int>
  {
  public:
    IntSetting(std::string path, int min, int max, int defaultValue, bool isReadOnly, bool isAdvanced);
    ~IntSetting() {}

    int getMinimum() const { return d_min; }
    int getMaximum() const { return d_max; }
    bool isValidValue(int value) const override;
    int getDefaultValue() const override { return d_defaultValue; }

  protected:
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
    EnumSetting(std::string path, std::map<int,std::string> pairs, int defaultValue, bool isReadOnly, bool isAdvanced);
    ~EnumSetting() {}

    bool isValidValue(int value) const override;
    int getDefaultValue() const override { return d_defaultValue; }

  protected:
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
    DoubleSetting(std::string path, double min, double max, double defaultValue, bool isReadOnly, bool isAdvanced);
    ~DoubleSetting() {}

    bool isValidValue(double value) const override;
    double getDefaultValue() const override { return d_defaultValue; }

  protected:
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
    BoolSetting(std::string path, bool defaultValue, bool isReadOnly, bool isAdvanced);
    ~BoolSetting() {}

    bool isValidValue(bool value) const override;
    bool getDefaultValue() const override { return d_defaultValue; }

  protected:
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

    HsvRangeSetting(std::string path, Colour::hsvRange defaultValue, bool isReadOnly, bool isAdvanced);
    ~HsvRangeSetting() {}

    bool isValidValue(Colour::hsvRange value) const override;
    Colour::hsvRange getDefaultValue() const override { return d_defaultValue; }

  protected:
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

    DoubleRangeSetting(std::string path, Range<double> defaultValue, bool isReadOnly, bool isAdvanced);
    ~DoubleRangeSetting() {}

    bool isValidValue(Range<double> value) const override;
    Range<double> getDefaultValue() const override { return d_defaultValue; }

  protected:
    virtual void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;
    virtual void writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    Range<double> d_defaultValue;
  };

  /// Models a setting with a std::string value.
  class StringSetting : public Setting<std::string>
  {
  public:
    StringSetting(std::string path, std::string defaultValue, bool isReadOnly, bool isAdvanced);
    ~StringSetting() {}

    bool isValidValue(std::string value) const override;
    std::string getDefaultValue() const override { return d_defaultValue; }

  protected:
    virtual void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;
    virtual void writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    std::string d_defaultValue;
  };
}
