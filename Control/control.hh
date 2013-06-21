#pragma once

#include <cassert>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace bold
{
  enum class ControlType
  {
    Unknown = 0,
    Int = 1,
    Bool = 2,
    Enum = 3,
    Action = 4
  };

  struct ControlEnumValue
  {
    ControlEnumValue() {}

    ControlEnumValue(int value, std::string name)
    : d_value(value),
      d_name(name)
    {}

    int getValue() const { return d_value; }
    std::string getName() const { return d_name; }

  private:
    static unsigned s_nextEnumValueId;

    int d_id;
    int d_value;
    std::string d_name;
  };

  class Control
  {
  public:
    static Control createInt(std::string name,
                             std::function<int()> getter,
                             std::function<void(int const& value)> callback);

    static Control createInt(unsigned id,
                             std::string name,
                             std::function<int()> getter,
                             std::function<void(int const& value)> callback);

    static Control createBool(std::string name,
                              std::function<bool()> getter,
                              std::function<void(bool const& value)> callback);

    static Control createBool(unsigned id,
                              std::string name,
                              std::function<bool()> getter,
                              std::function<void(bool const& value)> callback);

    static Control createEnum(std::string name,
                              std::vector<ControlEnumValue> enumValues,
                              std::function<unsigned()> getter,
                              std::function<void(ControlEnumValue const& value)> callback);

    static Control createEnum(unsigned id,
                              std::string name,
                              std::vector<ControlEnumValue> enumValues,
                              std::function<unsigned()> getter,
                              std::function<void(ControlEnumValue const& value)> callback);

    static Control createAction(std::string name,
                                std::function<void()> callback);

    static Control createAction(unsigned id,
                                std::string name,
                                std::function<void()> callback);

    Control()
    : d_type(ControlType::Unknown),
      d_getter(),
      d_setter(),
      d_name(),
      d_isAdvanced(false),
      d_hasLimitValues(false),
      d_minValue(0),
      d_maxValue(0),
      d_hasDefaultValue(false),
      d_defaultValue(0),
      d_enumValues()
    {}

    void setLimitValues(int const min, int const max)
    {
      assert(d_type == ControlType::Int);
      assert(min <= max);

      d_hasLimitValues = true;
      d_minValue = min;
      d_maxValue = max;
    }

    void setDefaultValue(int const defaultValue)
    {
      assert(d_type != ControlType::Unknown);
      assert(d_type != ControlType::Action);

      d_hasDefaultValue = true;
      d_defaultValue = defaultValue;
    }

    std::vector<ControlEnumValue> getEnumValues() const
    {
      assert(d_type == ControlType::Enum);
      return d_enumValues;
    }

    void setValue(int value)
    {
      assert(d_type != ControlType::Action);

      if (d_hasLimitValues && (value > d_maxValue || value < d_minValue))
      {
        std::cout << "[Control::setValue] Ignoring out-of-range value " << value << " for " << d_name << ". Must be between " << d_minValue << " and " << d_maxValue << "." << std::endl;
        return;
      }

      d_setter(value);

      if (d_getter)
      {
        // Test whether the value we set was taken or not
        int retrievedValue = d_getter();
        if (retrievedValue != value)
          std::cerr << "[Camera::setValue] Setting camera control with ID " << d_id << " failed -- set " << value << " but read back " << retrievedValue << std::endl;
      }
    }

    /** Handles a request against this control, received in JSON. */
    bool handleRequest(rapidjson::Document const& json);

    /** Provides the state of this control in JSON. */
    void writeState(rapidjson::Writer<rapidjson::StringBuffer>& writer) const;

    unsigned getId() const { return d_id; }
    int getValue() const { return d_getter(); }
    ControlType getType() const { return d_type; }
    std::string getName() const { return d_name; }
    bool isAdvanced() const { return d_isAdvanced; }
    void setIsAdvanced(bool isAdvanced) { d_isAdvanced = isAdvanced; }

    bool hasLimitValues() const { return d_hasLimitValues; }
    int getMinValue() const { return d_minValue; }
    int getMaxValue() const { return d_maxValue; }

    bool hasDefaultValue() const { return d_hasDefaultValue; }
    int getDefaultValue() const { return d_defaultValue; }

    friend std::ostream& operator<<(std::ostream& stream, Control const& control)
    {
      stream << control.getName() << "=" << control.getValue();

      switch (control.getType())
      {
        case bold::ControlType::Int: stream << " type=int"; break;
        case bold::ControlType::Bool: stream << " type=bool"; break;
        case bold::ControlType::Enum: stream << " type=enum"; break;
        case bold::ControlType::Action: stream << " type=action"; break;
        default:
          stream << " type=unknown"; break;
      }

      if (control.hasLimitValues())
        stream << " range=" << control.getMinValue() << "-" << control.getMaxValue();

      if (control.hasDefaultValue())
      {
        switch (control.getType())
        {
          case bold::ControlType::Bool:
          {
            stream << " default=" << (control.getDefaultValue() != 0 ? "true" : "false");
            break;
          }
          case bold::ControlType::Enum:
          {
            int index = control.getDefaultValue();
            auto values = control.getEnumValues();
            if (index >= 0 && index < values.size())
              stream << " default=" << (values[index].getName());
            else
              stream << " defaultIndex=" << control.getDefaultValue(); break;
            break;
          }
          default:
          {
            stream << " default=" << control.getDefaultValue(); break;
          }
        }
      }

      if (control.isAdvanced())
        stream << " advanced=true";

      return stream;
    }

  private:
    static unsigned s_nextControlId;

    unsigned d_id;
    ControlType d_type;
    std::function<int()> d_getter;
    std::function<void(int const& value)> d_setter;
    std::string d_name;
    bool d_isAdvanced;

    bool d_hasLimitValues;
    int d_minValue;
    int d_maxValue;

    bool d_hasDefaultValue;
    int d_defaultValue;

    std::vector<ControlEnumValue> d_enumValues;
  };
}
