#include "fsmoption.ih"

/*
 {
   "name": "win",
   "start": "startup",
   "states": [
     { "id": "startup" }
   ],
   "transitions": [
     { "from": "startup", "to": "ready", "label": "init" }
   ],
   "wildcardTransitions": [
     { "to": "ready", "label": "init" }
   ]
 }
*/

void FSMOption::toJson(Writer<StringBuffer>& writer) const
{
  writer.StartObject();
  {
    writer.String("name");
    writer.String(getId().c_str());
    writer.String("start");
    writer.String(d_startState->name.c_str());

    writer.String("states");
    writer.StartArray();
    {
      for (auto const& state : d_states)
      {
        writer.StartObject();
        {
          writer.String("id");
          writer.String(state->name.c_str());
        }
        writer.EndObject();
      }
    }
    writer.EndArray();

    writer.String("transitions");
    writer.StartArray();
    {
      for (auto const& state : d_states)
      {
        for (auto const& transition : state->transitions)
        {
          writer.StartObject();
          {
            writer.String("id");
            writer.String(transition->name.c_str());
            writer.String("from");
            writer.String(state->name.c_str());
            writer.String("to");
            writer.String(transition->childState->name.c_str());
          }
          writer.EndObject();
        }
      }
    }
    writer.EndArray();

    writer.String("wildcardTransitions");
    writer.StartArray();
    {
      for (auto const& transition : d_wildcardTransitions)
      {
        writer.StartObject();
        {
          writer.String("id");
          writer.String(transition->name.c_str());
          writer.String("to");
          writer.String(transition->childState->name.c_str());
        }
        writer.EndObject();
      }
    }
    writer.EndArray();
  }
  writer.EndObject();
}
