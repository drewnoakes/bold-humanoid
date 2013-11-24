#pragma once

#include <iostream>
#include <map>
#include <string>
#include <sigc++/signal.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>

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
    std::string getName() const { return d_name; }
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

    virtual bool setValueFromJson(rapidjson::Value* value) = 0;
    virtual void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const = 0;
    virtual void writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const = 0;

  protected:
    SettingBase(std::string path, std::string typeName, bool isReadOnly, bool isAdvanced)
    : d_path(path),
      d_typeName(typeName),
      d_isReadOnly(isReadOnly),
      d_isAdvanced(isAdvanced)
    {
      auto last = d_path.find_last_of('.');
      if (last == std::string::npos)
      {
        std::cerr << ccolor::error << "[SettingBase::SettingBase] Invalid path: " << d_path << ccolor::reset << std::endl;
        throw std::runtime_error("Invalid setting path");
      }
      d_name = d_path.substr(last + 1);
    }

    virtual ~SettingBase() {}

    static bool isInitialising();

  private:
    std::string d_name;
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
      if (isReadOnly() && !isInitialising())
      {
        std::cerr << ccolor::error << "[Setting::setValue] Attempt to modify readonly setting: " << getPath() << ccolor::reset << std::endl;
        return false;
      }

      if (!isValidValue(value))
      {
        std::cerr << ccolor::warning
                  << "[Setting::setValue] Attempt to set invalid value '" << value << "' to setting '" << getPath() << "': "
                  << getValidationMessage(value)
                  << ccolor::reset
                  << std::endl;
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

    virtual bool isValidValue(T value) const = 0;
    virtual std::string getValidationMessage(T value) const = 0;
    virtual T getDefaultValue() const = 0;

  private:
    T d_value;
  };
}
