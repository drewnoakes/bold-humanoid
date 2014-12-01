#pragma once

#include "../stateobject.hh"
#include "../../Drawing/drawing.hh"
#include "../../JsonWriter/jsonwriter.hh"

#include <memory>
#include <vector>

namespace bold
{
  class DrawingState : public StateObject
  {
  public:
    DrawingState(std::unique_ptr<std::vector<std::unique_ptr<DrawingItem const>>> items)
    : d_drawingItems(std::move(items))
    {}

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }

  private:
    template<typename TBuffer>
    inline void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    std::unique_ptr<std::vector<std::unique_ptr<DrawingItem const>>> d_drawingItems;
  };

  template<typename TBuffer>
  inline void DrawingState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
  {
    writer.StartObject();
    writer.String("items");
    writer.StartArray();

    for (std::unique_ptr<DrawingItem const> const& item : *d_drawingItems)
    {
      writer.StartObject();
      writer.String("type");
      writer.Uint((uint)item->type);
      writer.String("frame");
      writer.Uint((uint)item->frame);

      ASSERT((int)item->type != 0);

      // TODO refactor out common code, maybe with another base class

      if (item->type == DrawingItemType::Line)
      {
        auto line = static_cast<LineDrawing const*>(item.get());
        writer.String("p1");
        writer.StartArray();
        writer.Double(line->p1.x(), "%.3f");
        writer.Double(line->p1.y(), "%.3f");
        writer.EndArray();
        writer.String("p2");
        writer.StartArray();
        writer.Double(line->p2.x(), "%.3f");
        writer.Double(line->p2.y(), "%.3f");
        writer.EndArray();

        if (line->alpha > 0 && line->alpha < 1)
        {
          writer.String("a");
          writer.Double(line->alpha, "%.2f");
        }

        if (line->lineWidth > 0 && line->lineWidth != 1)
        {
          writer.String("w");
          writer.Double(line->lineWidth, "%.3f");
        }

        if (line->colour.b != 0 || line->colour.g != 0 || line->colour.r != 0)
        {
          writer.String("rgb");
          JsonWriter::rgb(writer, line->colour);
        }
      }
      else if (item->type == DrawingItemType::Circle)
      {
        auto circle = static_cast<CircleDrawing const*>(item.get());
        writer.String("c");
        writer.StartArray();
        writer.Double(circle->centre.x(), "%.3f");
        writer.Double(circle->centre.y(), "%.3f");
        writer.EndArray();
        writer.String("r");
        writer.Double(circle->radius, "%.3f");

        if (circle->fillAlpha > 0 && circle->fillAlpha < 1)
        {
          writer.String("fa");
          writer.Double(circle->fillAlpha, "%.2f");
        }
        if (circle->strokeAlpha > 0 && circle->strokeAlpha < 1)
        {
          writer.String("sa");
          writer.Double(circle->strokeAlpha, "%.2f");
        }
        if (circle->fillColour.b != 0 || circle->fillColour.g != 0 || circle->fillColour.r != 0)
        {
          writer.String("frgb");
          JsonWriter::rgb(writer, circle->fillColour);
        }
        if (circle->strokeColour.b != 0 || circle->strokeColour.g != 0 || circle->strokeColour.r != 0)
        {
          writer.String("srgb");
          JsonWriter::rgb(writer, circle->strokeColour);
        }
        if (circle->lineWidth > 0 && circle->lineWidth != 1)
        {
          writer.String("w");
          writer.Double(circle->lineWidth, "%.3f");
        }
      }
      else if (item->type == DrawingItemType::Polygon)
      {
        auto poly = static_cast<PolygonDrawing const*>(item.get());
        writer.String("p");
        JsonWriter::polygon(writer, poly->polygon, "%.3f");

        if (poly->fillAlpha > 0 && poly->fillAlpha < 1)
        {
          writer.String("fa");
          writer.Double(poly->fillAlpha, "%.2f");
        }
        if (poly->strokeAlpha > 0 && poly->strokeAlpha < 1)
        {
          writer.String("sa");
          writer.Double(poly->strokeAlpha, "%.2f");
        }
        if (poly->fillColour.b != 0 || poly->fillColour.g != 0 || poly->fillColour.r != 0)
        {
          writer.String("frgb");
          JsonWriter::rgb(writer, poly->fillColour);
        }
        if (poly->strokeColour.b != 0 || poly->strokeColour.g != 0 || poly->strokeColour.r != 0)
        {
          writer.String("srgb");
          JsonWriter::rgb(writer, poly->strokeColour);
        }
        if (poly->lineWidth > 0 && poly->lineWidth != 1)
        {
          writer.String("w");
          writer.Double(poly->lineWidth, "%.3f");
        }
      }

      writer.EndObject();
    }

    writer.EndArray();
    writer.EndObject();
  }
}
