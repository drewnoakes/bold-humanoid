#pragma once

#include <iostream>
#include <map>
#include <string>
#include <sigc++/signal.h>

#include "../PixelLabel/pixellabel.hh"
#include "../util/Range.hh"

namespace bold
{
  /// Models data common to all settings, irrespective of data type and other metadata.
  class SettingBase
  {
  public:
    std::string getPath() const { return d_path; }
    bool isReadOnly() const { return d_isReadOnly; }
    bool isAdvanced() const { return d_isAdvanced; }
    // TODO maybe return type_index instead
    std::string getTypeName() const { return d_typeName; }

  protected:
    SettingBase(std::string path, std::string typeName, bool isReadOnly, bool isAdvanced)
    : d_path(path),
      d_typeName(typeName),
      d_isReadOnly(isReadOnly),
      d_isAdvanced(isAdvanced)
    {}

    virtual ~SettingBase() {}

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
        std::cerr << "[Setting::setValue] Attempt to set invalid value '" << value << "' to setting: " << getPath() << std::endl;
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
    IntSetting(std::string path, int min, int max, int defaultValue, bool isReadOnly, bool isAdvanced)
    : Setting(path, "int", isReadOnly, isAdvanced, defaultValue),
      d_min(min),
      d_max(max),
      d_defaultValue(defaultValue)
    {}

    ~IntSetting() {}

    bool isValidValue(int value) const override
    {
      return value >= d_min && value <= d_max;
    }

    int getDefaultValue() const override { return d_defaultValue; }

  private:
    int d_min;
    int d_max;
    int d_defaultValue;
  };

  /// Models a setting with an integer value selected from a set of valid numbers.
  class EnumSetting : public Setting<int>
  {
  public:
    EnumSetting(std::string path, std::map<int,std::string> pairs, int defaultValue, bool isReadOnly, bool isAdvanced)
    : Setting(path, "enum", isReadOnly, isAdvanced, defaultValue),
      d_pairs(pairs),
      d_defaultValue(defaultValue)
    {}

    ~EnumSetting() {}

    bool isValidValue(int value) const override
    {
      return d_pairs.find(value) != d_pairs.end();
    }

    int getDefaultValue() const override { return d_defaultValue; }

  private:
    std::map<int,std::string> d_pairs;
    int d_defaultValue;
  };

  /// Models a setting with a double value.
  class DoubleSetting : public Setting<double>
  {
  public:
    DoubleSetting(std::string path, double min, double max, double defaultValue, bool isReadOnly, bool isAdvanced)
    : Setting(path, "double", isReadOnly, isAdvanced, defaultValue),
      d_min(min),
      d_max(max),
      d_defaultValue(defaultValue)
    {}

    ~DoubleSetting() {}

    bool isValidValue(double value) const override
    {
      return value >= d_min && value <= d_max;
    }

    double getDefaultValue() const override { return d_defaultValue; }

  private:
    double d_min;
    double d_max;
    double d_defaultValue;
  };

  /// Models a setting with a boolean value.
  class BoolSetting : public Setting<bool>
  {
  public:
    BoolSetting(std::string path, bool defaultValue, bool isReadOnly, bool isAdvanced)
    : Setting(path, "bool", isReadOnly, isAdvanced, defaultValue),
      d_defaultValue(defaultValue)
    {}

    ~BoolSetting() {}

    bool isValidValue(bool value) const override
    {
      return true;
    }

    bool getDefaultValue() const override { return d_defaultValue; }

  private:
    bool d_defaultValue;
  };

  /// Models a setting with a PixelLabel value.
  class PixelLabelSetting : public Setting<PixelLabel>
  {
  public:
    PixelLabelSetting(std::string path, PixelLabel defaultValue, bool isReadOnly, bool isAdvanced)
    : Setting(path, "pixel-label", isReadOnly, isAdvanced, defaultValue),
      d_defaultValue(defaultValue)
    {}

    ~PixelLabelSetting() {}

    bool isValidValue(PixelLabel value) const override
    {
      // TODO validate this!
      return true;
    }

    PixelLabel getDefaultValue() const override { return d_defaultValue; }

  private:
    PixelLabel d_defaultValue;
  };

  /// Models a setting with a Range<double> value.
  class DoubleRangeSetting : public Setting<Range<double>>
  {
  public:
    DoubleRangeSetting(std::string path, Range<double> defaultValue, bool isReadOnly, bool isAdvanced)
    : Setting(path, "double-range", isReadOnly, isAdvanced, defaultValue),
      d_defaultValue(defaultValue)
    {}

    ~DoubleRangeSetting() {}

    bool isValidValue(Range<double> value) const override
    {
      // TODO validate this!
      return true;
    }

    Range<double> getDefaultValue() const override { return d_defaultValue; }

  private:
    Range<double> d_defaultValue;
  };

  /// Models a setting with a std::string value.
  class StringSetting : public Setting<std::string>
  {
  public:
    StringSetting(std::string path, std::string defaultValue, bool isReadOnly, bool isAdvanced)
    : Setting(path, "string", isReadOnly, isAdvanced, defaultValue),
      d_defaultValue(defaultValue)
    {}

    ~StringSetting() {}

    bool isValidValue(std::string value) const override
    {
      // TODO validate this!
      return true;
    }

    std::string getDefaultValue() const override { return d_defaultValue; }

  private:
    std::string d_defaultValue;
  };
}
