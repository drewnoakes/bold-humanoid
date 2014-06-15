#include "labelcountstate.hh"

#include "../../PixelLabel/pixellabel.hh"

using namespace bold;
using namespace rapidjson;
using namespace std;

void LabelCountState::writeJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("labels");
    writer.StartArray();
    {
      for (auto const& pair : d_labelCounts)
      {
        shared_ptr<PixelLabel> const& label = pair.first;
        uint const& count = pair.second;
        writer.StartObject();
        {
          writer.String("name").String(label->getName().c_str());
          writer.String("id").Uint(label->getID());
          writer.String("count").Uint(count);
        }
        writer.EndObject();
      }
    }
    writer.EndArray();
  }
  writer.EndObject();
}
