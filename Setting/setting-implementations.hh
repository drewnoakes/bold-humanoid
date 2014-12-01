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
  class IntSetting final : public Setting<int>
  {
  public:
    IntSetting(std::string path, int min, int max, bool isReadOnly, std::string description);

    bool isValidValue(int const& value) const override;
    std::string getValidationMessage(int const& value) const override;
    bool tryParseJsonValue(rapidjson::Value const* jsonValue, int* parsedValue) const override;
    void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer, int const& value) const override { writeJsonInternal(writer, value); }
    void writeJsonValue(rapidjson::Writer<WebSocketBuffer>& writer, int const& value) const override { writeJsonInternal(writer, value); }
    void writeJsonValue(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, int const& value) const override { writeJsonInternal(writer, value); }
    void writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonMetadataInternal(writer); }
    void writeJsonMetadata(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonMetadataInternal(writer); }

  private:
    template<typename TBuffer>
    static void writeJsonInternal(rapidjson::Writer<TBuffer>& writer, int const& value)
    {
      writer.Int(value);
    }

    template<typename TBuffer>
    void writeJsonMetadataInternal(rapidjson::Writer<TBuffer>& writer) const
    {
      Setting<int>::writeJsonMetadata(writer);

      if (d_min != -std::numeric_limits<int>::max())
      {
        writer.String("min");
        writer.Int(d_min);
      }

      if (d_max != std::numeric_limits<int>::max())
      {
        writer.String("max");
        writer.Int(d_max);
      }
    }

    int d_min;
    int d_max;
  };

  /// Models a setting with an integer value selected from a set of valid numbers.
  class EnumSetting final : public Setting<int>
  {
  public:
    EnumSetting(std::string path, std::map<int,std::string> pairs, bool isReadOnly, std::string description);

    bool isValidValue(int const& value) const override;
    std::string getValidationMessage(int const& value) const override;
    bool tryParseJsonValue(rapidjson::Value const* jsonValue, int* parsedValue) const override;
    void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer, int const& value) const override { writeJsonInternal(writer, value); }
    void writeJsonValue(rapidjson::Writer<WebSocketBuffer>& writer, int const& value) const override { writeJsonInternal(writer, value); }
    void writeJsonValue(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, int const& value) const override { writeJsonInternal(writer, value); }
    void writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonMetadataInternal(writer); }
    void writeJsonMetadata(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonMetadataInternal(writer); }

  private:
    template<typename TBuffer>
    static void writeJsonInternal(rapidjson::Writer<TBuffer>& writer, int const& value)
    {
      writer.Int(value);
    }

    template<typename TBuffer>
    void writeJsonMetadataInternal(rapidjson::Writer<TBuffer>& writer) const
    {
      Setting<int>::writeJsonMetadata(writer);

      writer.String("values");
      writer.StartArray();
      {
        for (auto const& pair : d_pairs)
        {
          writer.StartObject();
          writer.String("text");
          writer.String(pair.second.c_str());
          writer.String("value");
          writer.Int(pair.first);
          writer.EndObject();
        }
      }
      writer.EndArray();
    }

    std::map<int,std::string> d_pairs;
  };

  /// Models a setting with a double value.
  class DoubleSetting final : public Setting<double>
  {
  public:
    DoubleSetting(std::string path, double min, double max, bool isReadOnly, std::string description);

    bool isValidValue(double const& value) const override;
    std::string getValidationMessage(double const& value) const override;
    bool tryParseJsonValue(rapidjson::Value const* jsonValue, double* parsedValue) const override;
    void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer, double const& value) const override { writeJsonInternal(writer, value); }
    void writeJsonValue(rapidjson::Writer<WebSocketBuffer>& writer, double const& value) const override { writeJsonInternal(writer, value); }
    void writeJsonValue(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, double const& value) const override { writeJsonInternal(writer, value); }
    void writeJsonMetadata(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonMetadataInternal(writer); }
    void writeJsonMetadata(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonMetadataInternal(writer); }

  private:
    template<typename TBuffer>
    static void writeJsonInternal(rapidjson::Writer<TBuffer>& writer, double const& value)
    {
      writer.Double(value);
    }

    template<typename TBuffer>
    void writeJsonMetadataInternal(rapidjson::Writer<TBuffer>& writer) const
    {
      Setting<double>::writeJsonMetadata(writer);

      if (d_min != -std::numeric_limits<double>::max())
      {
        writer.String("min");
        writer.Double(d_min);
      }
      if (d_max != std::numeric_limits<double>::max())
      {
        writer.String("max");
        writer.Double(d_max);
      }
    }

    double d_min;
    double d_max;
  };

  /// Models a setting with a boolean value.
  class BoolSetting final : public Setting<bool>
  {
  public:
    BoolSetting(std::string path, bool isReadOnly, std::string description);

    bool tryParseJsonValue(rapidjson::Value const* jsonValue, bool* parsedValue) const override;
    void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer, bool const& value) const override { writeJsonInternal(writer, value); }
    void writeJsonValue(rapidjson::Writer<WebSocketBuffer>& writer, bool const& value) const override { writeJsonInternal(writer, value); }
    void writeJsonValue(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, bool const& value) const override { writeJsonInternal(writer, value); }

  private:
    template<typename TBuffer>
    static void writeJsonInternal(rapidjson::Writer<TBuffer>& writer, bool const& value)
    {
      writer.Bool(value);
    }
  };

  /// Models a setting with a Colour::hsvRange value.
  class HsvRangeSetting final : public Setting<Colour::hsvRange>
  {
  public:
    HsvRangeSetting(std::string path, bool isReadOnly, std::string description);

    bool isValidValue(Colour::hsvRange const& value) const override;
    std::string getValidationMessage(Colour::hsvRange const& value) const override;
    bool tryParseJsonValue(rapidjson::Value const* jsonValue, Colour::hsvRange* parsedValue) const override;
    void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer, Colour::hsvRange const& value) const override { writeJsonInternal(writer, value); }
    void writeJsonValue(rapidjson::Writer<WebSocketBuffer>& writer, Colour::hsvRange const& value) const override { writeJsonInternal(writer, value); }
    void writeJsonValue(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Colour::hsvRange const& value) const override { writeJsonInternal(writer, value); }

  private:
    template<typename TBuffer>
    static void writeJsonInternal(rapidjson::Writer<TBuffer>& writer, Colour::hsvRange const& value)
    {
      writer.StartObject();
      {
        writer.String("hue");
        writer.StartArray();
        writer.Uint(value.hMin);
        writer.Uint(value.hMax);
        writer.EndArray();

        writer.String("sat");
        writer.StartArray();
        writer.Uint(value.sMin);
        writer.Uint(value.sMax);
        writer.EndArray();

        writer.String("val");
        writer.StartArray();
        writer.Uint(value.vMin);
        writer.Uint(value.vMax);
        writer.EndArray();
      }
      writer.EndObject();
    }
  };

  /// Models a setting with a Range<double> value.
  class DoubleRangeSetting final : public Setting<Range<double>>
  {
  public:
    DoubleRangeSetting(std::string path, bool isReadOnly, std::string description);

    bool isValidValue(Range<double> const& value) const override;
    std::string getValidationMessage(Range<double> const& value) const override;
    bool tryParseJsonValue(rapidjson::Value const* jsonValue, Range<double>* parsedValue) const override;
    void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer, Range<double> const& value) const override { writeJsonInternal(writer, value); }
    void writeJsonValue(rapidjson::Writer<WebSocketBuffer>& writer, Range<double> const& value) const override { writeJsonInternal(writer, value); }
    void writeJsonValue(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Range<double> const& value) const override { writeJsonInternal(writer, value); }

  private:
    template<typename TBuffer>
    static void writeJsonInternal(rapidjson::Writer<TBuffer>& writer, Range<double> const& value)
    {
      writer.StartArray();
      writer.Double(value.min());
      writer.Double(value.max());
      writer.EndArray();
    }
  };

  /// Models a setting with a std::string value.
  class StringSetting final : public Setting<std::string>
  {
  public:
    StringSetting(std::string path, bool isReadOnly, std::string description);

    bool isValidValue(std::string const& value) const override;
    std::string getValidationMessage(std::string const& value) const override;
    bool tryParseJsonValue(rapidjson::Value const* jsonValue, std::string* parsedValue) const override;
    void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer, std::string const& value) const override { writeJsonInternal(writer, value); }
    void writeJsonValue(rapidjson::Writer<WebSocketBuffer>& writer, std::string const& value) const override { writeJsonInternal(writer, value); }
    void writeJsonValue(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, std::string const& value) const override { writeJsonInternal(writer, value); }

  private:
    template<typename TBuffer>
    static void writeJsonInternal(rapidjson::Writer<TBuffer>& writer, std::string const& value)
    {
      writer.String(value.c_str());
    }
  };

  /// Models a setting with a std::string value.
  class StringArraySetting final : public Setting<std::vector<std::string>>
  {
  public:
    StringArraySetting(std::string path, bool isReadOnly, std::string description);

    bool areValuesEqual(std::vector<std::string> const& a, std::vector<std::string> const& b) const override;
    bool tryParseJsonValue(rapidjson::Value const* jsonValue, std::vector<std::string>* parsedValue) const override;
    void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer, std::vector<std::string> const& value) const override { writeJsonInternal(writer, value); }
    void writeJsonValue(rapidjson::Writer<WebSocketBuffer>& writer, std::vector<std::string> const& value) const override { writeJsonInternal(writer, value); }
    void writeJsonValue(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, std::vector<std::string> const& value) const override { writeJsonInternal(writer, value); }

  private:
    template<typename TBuffer>
    static void writeJsonInternal(rapidjson::Writer<TBuffer>& writer, std::vector<std::string> const& value)
    {
      writer.StartArray();
      {
        for (auto const& s : value)
          writer.String(s.c_str());
      }
      writer.EndArray();
    }
  };

  /// Models a setting with a Colour::bgr value.
  class BgrColourSetting final : public Setting<Colour::bgr>
  {
  public:
    BgrColourSetting(std::string path, bool isReadOnly, std::string description);

    bool tryParseJsonValue(rapidjson::Value const* jsonValue, Colour::bgr* parsedValue) const override;
    void writeJsonValue(rapidjson::Writer<rapidjson::StringBuffer>& writer, Colour::bgr const& value) const override { writeJsonInternal(writer, value); }
    void writeJsonValue(rapidjson::Writer<WebSocketBuffer>& writer, Colour::bgr const& value) const override { writeJsonInternal(writer, value); }
    void writeJsonValue(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, Colour::bgr const& value) const override { writeJsonInternal(writer, value); }

  private:
    template<typename TBuffer>
    static void writeJsonInternal(rapidjson::Writer<TBuffer>& writer, Colour::bgr const& value)
    {
      writer.StartObject();
      {
        writer.String("r");
        writer.Int(value.r);
        writer.String("g");
        writer.Int(value.g);
        writer.String("b");
        writer.Int(value.b);
      }
      writer.EndObject();
    }
  };
}
