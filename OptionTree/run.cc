#include "optiontree.ih"

#include <functional>
#include <rapidjson/document.h>

using namespace rapidjson;

/*
 {
   "name": "win",
   "type": "FSM",
   "run": {
     "start": "playing",
     "transitions": [
       { "to": "jumping", "via": "see-elevated-carrot" },
       { "to": "crying", "via": "carrot-too-high" }
     ]
   },
   "children": [
     {
       "name": "wipe-eyes",
       "type": "MotionScript",
       "run": {
         "motionScript": "wipe-eyes.json"
       }
     }
   ]
 }
*/

void OptionTree::run()
{
  vector<shared_ptr<Option>> ranOptions;

  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);

  list<shared_ptr<Option>> queue = {d_top};

  function<void(shared_ptr<Option>)> runOption;
  runOption = [&runOption,&writer,&ranOptions](shared_ptr<Option> option)
  {
    log::verbose("OptionTree::run") << "Running " << option->getTypeName() << " option: " << option->getId();

    writer.StartObject();

    writer.String("id").String(option->getId().c_str());
    writer.String("type").String(option->getTypeName().c_str());

    // Run it
    writer.String("run").StartObject();
    vector<shared_ptr<Option>> subOptions = option->runPolicy(writer);
    writer.EndObject();

    if (!subOptions.empty())
    {
      writer.String("children");
      writer.StartArray();
      for (auto const& child : subOptions)
        runOption(child);
      writer.EndArray();
    }

    writer.EndObject();

    // Remember the fact that we ran it
    ranOptions.push_back(option);
  };

  runOption(d_top);

  // NOTE this is a bit weird/wasteful... we should generate the Document
  //      above and pass it instead of creating the string then parsing it...
  unique_ptr<Document> doc(new Document());
  doc->Parse<0,UTF8<>>(buffer.GetString());

  State::set(make_shared<OptionTreeState const>(ranOptions, std::move(doc)));
}
