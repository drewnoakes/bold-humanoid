#pragma once

#include <iostream>
#include <map>
#include <string>
#include <sigc++/signal.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>
#include <typeindex>

#include "../PixelLabel/pixellabel.hh"
#include "../util/Range.hh"
#include "../util/ccolor.hh"
#include "../util/log.hh"

namespace bold
{
  /// Models data common to all settings, irrespective of data type and other metadata.
  class SettingBase
  {
  public:
    sigc::signal<void, SettingBase*> changedBase;

    std::string getPath() const { return d_path; }
    std::string getName() const { return d_name; }
    bool isReadOnly() const { return d_isReadOnly; }
    bool isAdvanced() const { return d_isAdvanced; }
    std::string getTypeName() const { return d_typeName; }
    std::type_index getTypeIndex() const { return d_typeIndex; }
    std::string getDescription() const { return d_description; }

    void writeFullJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const;

    virtual void resetToDefaultValue() = 0;
    virtual bool setValueFromJson(rapidjson::Value const* value) = 0;
    virtual void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const = 0;
    virtual void writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const = 0;

  protected:
    SettingBase(std::string path, std::string typeName, std::type_index typeIndex, bool isReadOnly, bool isAdvanced, std::string description)
    : d_path(path),
      d_typeName(typeName),
      d_typeIndex(typeIndex),
      d_isReadOnly(isReadOnly),
      d_isAdvanced(isAdvanced),
      d_description(description)
    {
      auto last = d_path.find_last_of('.');
      auto nameStart = 
        last == std::string::npos ?
        0 : last + 1;
      d_name = d_path.substr(nameStart);
    }

    virtual ~SettingBase() {}

    static bool isInitialising();

  private:
    std::string d_name;
    std::string d_path;
    std::string d_typeName;
    std::type_index d_typeIndex;
    bool d_isReadOnly;
    bool d_isAdvanced;
    std::string d_description;
  };

  /// Abstract model of a setting with a particular data type.
  /// Subclasses implement specific validation logic and serialisation.
  template<typename T>
  class Setting : public SettingBase
  {
  public:
    Setting(std::string path, std::string typeName, bool isReadOnly, bool isAdvanced, T value, std::string description)
    : SettingBase(path, typeName, typeid(T), isReadOnly, isAdvanced, description),
      d_value(value)
    {}

    virtual ~Setting() {}

    const T getValue() const { return d_value; }

    bool setValue(T value)
    {
      if (isReadOnly() && !isInitialising())
      {
        log::error("Setting::setValue") << "Attempt to modify readonly setting: " << getPath();
        return false;
      }

      if (!isValidValue(value))
      {
        log::error("Setting::setValue") << "Attempt to set invalid value '" << value << "' to setting '" << getPath() << "': " << getValidationMessage(value);
        return false;
      }

      d_value = value;
      changed(value);
      changedBase(this);
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

    void resetToDefaultValue() override
    {
      setValue(getDefaultValue());
    }

    virtual bool isValidValue(T value) const = 0;
    virtual std::string getValidationMessage(T value) const = 0;
    virtual T getDefaultValue() const = 0;

  private:
    T d_value;
  };
}
