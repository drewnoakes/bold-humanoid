#include "control.ih"

void Control::writeState(Writer<StringBuffer>& writer) const
{
  writer.String("name");
  writer.String(d_name.c_str());

  writer.String("id");
  writer.Uint(d_id);

  writer.String("type");
  writer.Int((unsigned)d_type);

  if (d_type == ControlType::Bool)
  {
    writer.String("value");
    writer.Bool(d_value != 0);
  }
  else if (d_type == ControlType::Int || d_type == ControlType::Enum)
  {
    writer.String("value");
    writer.Int(d_value);
  }

  if (d_hasLimitValues)
  {
    writer.String("minimum");
    writer.Int(d_minValue);

    writer.String("maximum");
    writer.Int(d_maxValue);
  }

  if (d_hasDefaultValue)
  {
    writer.String("defaultValue");
    writer.Int(d_defaultValue);
  }

  if (d_type == ControlType::Enum)
  {
    writer.String("enumValues");
    writer.StartArray();
    {
      for (auto& enumValue : d_enumValues)
      {
        writer.StartObject();
        {
          writer.String("value");
          writer.Uint(enumValue.getValue());

          writer.String("name");
          writer.String(enumValue.getName().c_str());
        }
        writer.EndObject();
      }
    }
    writer.EndArray();
  }
}
