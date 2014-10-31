#pragma once

#include "../Colour/colour.hh"
#include "../geometry/Polygon2.hh"

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace bold
{
  class JsonWriter
  {
  public:
    template<typename TBuffer>
    static void rgb(rapidjson::Writer<TBuffer>& writer, Colour::bgr const& bgr)
    {
      writer.StartArray();
      writer.Uint(bgr.r);
      writer.Uint(bgr.g);
      writer.Uint(bgr.b);
      writer.EndArray();
    }

    template<typename TBuffer>
    static void polygon(rapidjson::Writer<TBuffer>& writer, Polygon2d const& poly)
    {
      writer.StartArray();
      for (auto const& point : poly)
      {
        writer.StartArray();
        writer.Double(point.x());
        writer.Double(point.y());
        writer.EndArray();
      }
      writer.EndArray();
    }

    template<typename TBuffer>
    static void swapNaN(rapidjson::Writer<TBuffer>& writer, double d)
    {
      if (std::isnan(d))
        writer.Null();
      else
        writer.Double(d, "%.5f");
    };

  private:
    JsonWriter() = delete;
  };
}
