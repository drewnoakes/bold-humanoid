#pragma once

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
  /// Models data common to all settings, irrespective of data type and other metadata.
  class SettingBase
  {
  public:
    std::string getPath() const { return d_path; }
    bool isReadOnly() const { return d_isReadOnly; }
    bool isAdvanced() const { return d_isAdvanced; }
    std::string getTypeName() const { return d_typeName; }

    // TODO maybe return type_index as well, to allow validation

    void writeFullJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
    {
      writer.StartObject();
      {
        writer.String("path").String(getPath().c_str());
        writer.String("type").String(getTypeName().c_str());
        if (isAdvanced())
          writer.String("advanced").Bool(true);
        if (isReadOnly())
          writer.String("readonly").Bool(true);
        writer.String("value");
        writeJsonValue(writer);
        writeJsonMetadata(writer);
      }
      writer.EndObject();
    }

  protected:
    SettingBase(std::string path, std::string typeName, bool isReadOnly, bool isAdvanced)
    : d_path(path),
      d_typeName(typeName),
      d_isReadOnly(isReadOnly),
      d_isAdvanced(isAdvanced)
    {}

    virtual ~SettingBase() {}

    virtual void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const = 0;
    virtual void writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const = 0;

  private:
    std::string d_path;
    std::string d_typeName;
    bool d_isReadOnly;
    bool d_isAdvanced;
  };

  /// Abstract model of a setting with a particular data type.
  /// Subclasses implement specific validation logic and serialisation.
  template<typename T>
  class Setting : public SettingBase
  {
  public:
    Setting(std::string path, std::string typeName, bool isReadOnly, bool isAdvanced, T value)
    : SettingBase(path, typeName, isReadOnly, isAdvanced),
      d_value(value)
    {}

    virtual ~Setting() {}

    const T getValue() const { return d_value; }

    bool setValue(T value)
    {
      if (isReadOnly())
      {
        std::cerr << "[Setting::setValue] Attempt to modify readonly setting: " << getPath() << std::endl;
        return false;
      }

      if (!isValidValue(value))
      {
        std::cerr << ccolor::fore::lightred << "[Setting::setValue] Attempt to set invalid value '" << value << "' to setting: " << getPath() << ccolor::reset << std::endl;
        return false;
      }

      d_value = value;
      changed(value);
      return true;
    }

    /// Fires when the setting's value is assigned.
    sigc::signal<void, T const&> changed;

    /// Invokes the callback immediately with the current value, and notifies
    /// of any subsequent changes.
    void track(std::function<void(T)> callback)
    {
      changed.connect(callback);
      callback(d_value);
    }

    //virtual T setValueFromJson(JsonReader* reader) = 0;
    //virtual void writeValueToJson(JsonWriter* writer) const = 0;
    virtual bool isValidValue(T value) const = 0;
    virtual T getDefaultValue() const = 0;

  private:
    T d_value;
  };

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

  /// Models a setting with a PixelLabel value.
  class PixelLabelSetting : public Setting<PixelLabel>
  {
  public:
    static void writeHsvRangeJsonObject(rapidjson::Writer<rapidjson::StringBuffer>& writer, Colour::hsvRange const& value);

    PixelLabelSetting(std::string path, PixelLabel defaultValue, bool isReadOnly, bool isAdvanced);
    ~PixelLabelSetting() {}

    bool isValidValue(PixelLabel value) const override;
    PixelLabel getDefaultValue() const override { return d_defaultValue; }

  protected:
    virtual void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;
    virtual void writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    PixelLabel d_defaultValue;
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
