#include "drawingstate.hh"

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

    writer.EndObject();
  }

  writer.EndArray();
  writer.EndObject();
}