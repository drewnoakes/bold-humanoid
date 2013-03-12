#ifndef BOLD_CONTROL_COMMAND_HH
#define BOLD_CONTROL_COMMAND_HH

#include <string>
#include <vector>
#include <functional>
#include <cassert>
#include <iostream>

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>

namespace bold
{
  enum class ControlType
  {
    Unknown = 0,
    Int = 1,
    Bool = 2,
    Enum = 3,
    Action = 4
//     Int64 = 5,
//     ControlClass = 6,
//     String = 7,
//     BitMask = 8
  };

  struct ControlEnumValue
  {
    ControlEnumValue() {}

    ControlEnumValue(int value, std::string name)
    : d_value(value),
      d_name(name)
    {
//       d_id = s_nextEnumValueId++;
    }

//     ControlEnumValue(int id, int value, std::string name)
//     : d_id(id),
//       d_value(value),
//       d_name(name)
//     {}

//    int getId() const { return d_id; }
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
                             int value,
                             std::function<void(int const& value)> callback);

    static Control createInt(unsigned id,
                             std::string name,
                             int value,
                             std::function<void(int const& value)> callback);

    static Control createBool(std::string name,
                              bool value,
                              std::function<void(bool const& value)> callback);

    static Control createBool(unsigned id,
                              std::string name,
                              bool value,
                              std::function<void(bool const& value)> callback);

    static Control createEnum(std::string name,
                              std::vector<ControlEnumValue> enumValues,
                              unsigned value,
                              std::function<void(ControlEnumValue const& value)> callback);

    static Control createEnum(unsigned id,
                              std::string name,
                              std::vector<ControlEnumValue> enumValues,
                              unsigned value,
                              std::function<void(ControlEnumValue const& value)> callback);

    static Control createAction(std::string name,
                                std::function<void()> callback);

    static Control createAction(unsigned id,
                                std::string name,
                                std::function<void()> callback);

    Control()
    : d_callback(),
      d_value(0),
      d_type(ControlType::Unknown),
      d_name(),
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

//     int getValue() const { return d_value; };

    void setValue(int value)
    {
      assert(d_type != ControlType::Action /* && d_type != ControlType::ControlClass && d_type != ControlType::String*/);
      d_value = value;
      d_callback(d_value);
    }

    /** Handles a request against this control, received in JSON. */
    bool handleRequest(rapidjson::Document const& json);

    /** Provides the state of this control in JSON. */
    void writeState(rapidjson::Writer<rapidjson::StringBuffer>& writer) const;

    unsigned getId() const { return d_id; }
    int getValue() const { return d_value; }
    ControlType getType() const { return d_type; }
    std::string getName() const { return d_name; }

    bool hasLimitValues() const { return d_hasLimitValues; }
    int getMinValue() const { return d_minValue; }
    int getMaxValue() const { return d_maxValue; }

    bool hasDefaultValue() const { return d_hasDefaultValue; }
    int getDefaultValue() const { return d_defaultValue; }

    friend std::ostream& operator<<(std::ostream& stream, Control const& control)
    {
      stream << control.getName() << "(" << (int)control.getType() << ")" << "=" << control.getValue();

      if (control.hasLimitValues())
        stream << " (" << control.getMinValue() << "-" << control.getMaxValue() << ")";

      if (control.hasDefaultValue())
        stream << "[" << control.getDefaultValue() << "]";

      return stream;
    }

  private:
    static unsigned s_nextControlId;

    unsigned d_id;
    int d_value;
    ControlType d_type;
    std::function<void(int const& value)> d_callback;
    std::string d_name;

    bool d_hasLimitValues;
    int d_minValue;
    int d_maxValue;

    bool d_hasDefaultValue;
    int d_defaultValue;

    std::vector<ControlEnumValue> d_enumValues;
  };
}

#endif