#include "drawingstate.hh"

#include "../../JsonWriter/jsonwriter.hh"
#include "../../util/assert.hh"

using namespace std;
using namespace bold;
using namespace rapidjson;

void DrawingState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  writer.String("items");
  writer.StartArray();

  for (unique_ptr<DrawingItem const> const& item : *d_drawingItems)
  {
    writer.StartObject();
    writer.String("type").Uint((uint)item->type);
    writer.String("frame").Uint((uint)item->frame);

    ASSERT((int)item->type != 0);

    // TODO refactor out common code, maybe with another base class

    if (item->type == DrawingItemType::Line)
    {
      auto line = static_cast<LineDrawing const*>(item.get());
      writer.String("p1").StartArray().Double(line->p1.x()).Double(line->p1.y()).EndArray();
      writer.String("p2").StartArray().Double(line->p2.x()).Double(line->p2.y()).EndArray();

      if (line->alpha > 0 && line->alpha < 1)
        writer.String("a").Double(line->alpha);

      if (line->lineWidth > 0 && line->lineWidth != 1)
        writer.String("w").Double(line->lineWidth);

      if (line->colour.b != 0 || line->colour.g != 0 || line->colour.r != 0)
      {
        writer.String("rgb")
          .StartArray()
          .Uint(line->colour.r)
          .Uint(line->colour.g)
          .Uint(line->colour.b)
          .EndArray();
      }
    }
    else if (item->type == DrawingItemType::Circle)
    {
      auto circle = static_cast<CircleDrawing const*>(item.get());
      writer.String("c").StartArray().Double(circle->centre.x()).Double(circle->centre.y()).EndArray();
      writer.String("r").Double(circle->radius);

      if (circle->alpha > 0 && circle->alpha < 1)
        writer.String("a").Double(circle->alpha);

      if (circle->lineWidth > 0 && circle->lineWidth != 1)
        writer.String("w").Double(circle->lineWidth);

      if (circle->colour.b != 0 || circle->colour.g != 0 || circle->colour.r != 0)
      {
        writer.String("rgb")
          .StartArray()
          .Uint(circle->colour.r)
          .Uint(circle->colour.g)
          .Uint(circle->colour.b)
          .EndArray();
      }
    }
    else if (item->type == DrawingItemType::Polygon)
    {
      auto poly = static_cast<PolygonDrawing const*>(item.get());
      writer.String("p");
      JsonWriter::polygon(writer, poly->polygon);

      if (poly->fillAlpha > 0 && poly->fillAlpha < 1)
        writer.String("fa").Double(poly->fillAlpha);
      if (poly->strokeAlpha > 0 && poly->strokeAlpha < 1)
        writer.String("sa").Double(poly->strokeAlpha);
      if (poly->lineWidth > 0 && poly->lineWidth != 1)
        writer.String("w").Double(poly->lineWidth);

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
    }

    writer.EndObject();
  }

  writer.EndArray();
  writer.EndObject();
}
