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
    static void rgb(rapidjson::Writer<rapidjson::StringBuffer>& writer, Colour::bgr const& bgr)
    {
      writer
        .StartArray()
        .Uint(bgr.r)
        .Uint(bgr.g)
        .Uint(bgr.b)
        .EndArray();
    }

    static void polygon(rapidjson::Writer<rapidjson::StringBuffer>& writer, Polygon2d const& poly)
    {
      writer.StartArray();
      for (auto const& point : poly)
        writer.StartArray().Double(point.x()).Double(point.y()).EndArray();
      writer.EndArray();
    }

  private:
    JsonWriter() = delete;
  };
}
