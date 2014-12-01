#pragma once

#include <map>
#include <string>
#include <sigc++/signal.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>
#include <typeindex>
#include <rapidjson/prettywriter.h>

#include "../PixelLabel/pixellabel.hh"
#include "../util/Range.hh"
#include "../util/ccolor.hh"
#include "../util/log.hh"
#include "../util/websocketbuffer.hh"

namespace bold
{
  /// Models data common to all settings, irrespective of data type and other metadata.
  class SettingBase
  {
  public:
    sigc::signal<void, SettingBase const*> changedBase;

    std::string getPath() const { return d_path; }
    std::string getName() const { return d_name; }
    bool isReadOnly() const { return d_isReadOnly; }
    std::string getTypeName() const { return d_typeName; }
    std::type_index getTypeIndex() const { return d_typeIndex; }
    std::string getDescription() const { return d_description; }

    void writeFullJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const;
    void writeFullJson(rapidjson::Writer<WebSocketBuffer>& writer) const;

    virtual void triggerChanged() const;

    virtual bool isModified() const = 0;
    virtual void resetToInitialValue() = 0;
    virtual bool setValueFromJson(rapidjson::Value const* jsonValue) = 0;
    virtual void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const = 0;
    virtual void writeJsonValue(rapidjson::Writer<WebSocketBuffer>& writer) const = 0;
    virtual void writeJsonValue(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const = 0;
    virtual void writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const = 0;
    virtual void writeJsonMetadata(rapidjson::Writer<WebSocketBuffer>& writer) const = 0;

  protected:
    SettingBase(std::string path, std::string typeName, std::type_index typeIndex, bool isReadOnly, std::string description);

    virtual ~SettingBase() = default;

    static bool isInitialising();

    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer>& writer) const
    {
      writer.StartObject();
      {
        writer.String("path");
        writer.String(getPath().c_str());
        writer.String("type");
        writer.String(getTypeName().c_str());
        if (getDescription().size())
        {
          writer.String("description");
          writer.String(getDescription().c_str());
        }
        if (isReadOnly())
        {
          writer.String("readonly");
          writer.Bool(true);
        }
        writer.String("value");
        writeJsonValue(writer);
        writeJsonMetadata(writer);
      }
      writer.EndObject();
    }

  private:
    std::string d_name;
    std::string d_path;
    std::string d_typeName;
    std::type_index d_typeIndex;
    bool d_isReadOnly;
    std::string d_description;
  };

  /// Abstract model of a setting with a particular data type.
  /// Subclasses implement specific validation logic and serialisation.
  template<typename T>
  class Setting : public SettingBase
  {
  public:
    Setting(std::string path, std::string typeName, bool isReadOnly, std::string description)
    : SettingBase(path, typeName, typeid(T), isReadOnly, description),
      d_isInitialValueSet(false)
    {}

    virtual ~Setting() = default;

    const T getValue() const { return d_value; }

    const T getInitialValue() const
    {
      ASSERT(d_isInitialValueSet);
      return d_initialValue;
    }

    bool isModified() const override { return !areValuesEqual(d_value, d_initialValue); }

    bool setValueFromJson(rapidjson::Value const* value) override
    {
      if (value == nullptr)
      {
        log::error("Setting::setValueFromJson") << "Null JSON Value provided for: " << getPath();
        return false;
      }

      T parsedValue;
      if (!tryParseJsonValue(value, &parsedValue))
      {
        log::error("Setting::setValueFromJson") << "Unable to parse value for: " << getPath();
        return false;
      }

      return setValue(parsedValue);
    }

    bool setValue(T const& value)
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

      if (!d_isInitialValueSet)
      {
        if (!isInitialising())
        {
          log::error("Setting::setValue") << "Attempt to set initial value after initialisation has completed.";
          throw std::runtime_error("Attempt to set initial value after initialisation has completed.");
        }

        d_initialValue = value;
        d_isInitialValueSet = true;
      }

      d_value = value;

      triggerChanged();

      return true;
    }

    /// Fires when the setting's value is changed.
    sigc::signal<void, T const&> changed;

    void triggerChanged() const override
    {
      changed(d_value);
      SettingBase::triggerChanged();
    }

    /// Invokes the provided callback immediately with the current value, and notifies of any subsequent changes.
    void track(std::function<void(T)> callback)
    {
      changed.connect(callback);
      callback(d_value);
    }

    void resetToInitialValue() override
    {
      ASSERT(d_isInitialValueSet);
      setValue(d_initialValue);
    }

    virtual bool areValuesEqual(T const& a, T const& b) const { return a == b; };
    virtual bool isValidValue(T const& value) const { return true; };
    virtual std::string getValidationMessage(T const& value) const { return ""; }
    virtual bool tryParseJsonValue(rapidjson::Value const* jsonValue, T* parsedValue) const = 0;
    virtual void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer, T const& value) const = 0;
    virtual void writeJsonValue(rapidjson::Writer<WebSocketBuffer>& writer, T const& value) const = 0;
    virtual void writeJsonValue(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, T const& value) const = 0;

    void writeJsonValue(rapidjson::Writer<WebSocketBuffer>& writer) const
    {
      writeJsonValue(writer, getValue());
    }

    void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
    {
      writeJsonValue(writer, getValue());
    }

    void writeJsonValue(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const
    {
      writeJsonValue(writer, getValue());
    }

    virtual void writeJsonMetadata(rapidjson::Writer<WebSocketBuffer>& writer) const
    {
      writeJsonMetadataInternal(writer);
    }

    virtual void writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const
    {
      writeJsonMetadataInternal(writer);
    }

  private:
    template<typename TBuffer>
    void writeJsonMetadataInternal(rapidjson::Writer<TBuffer>& writer) const
    {
      ASSERT(d_isInitialValueSet);

      writer.String("initial");
      writeJsonValue(writer, d_initialValue);
    }

    T d_value;
    T d_initialValue;
    bool d_isInitialValueSet;
  };
}

