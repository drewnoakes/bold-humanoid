#include "optiontree.ih"

#include "../Option/FSMOption/fsmoption.hh"

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
  vector<FSMStateSnapshot> fsmStates;

  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);

  function<void(shared_ptr<Option>)> runOption;
  runOption = [this,&runOption,&writer,&ranOptions,&fsmStates](shared_ptr<Option> option)
  {
    log::verbose("OptionTree::run") << "Running " << option->getTypeName() << " option: " << option->getId();

    writer.StartObject();

    writer.String("id").String(option->getId().c_str());
    writer.String("type").String(option->getTypeName().c_str());

    // Remove options that run from the set of last cycle
    auto it = d_optionsLastCycle.find(option);
    if (it != d_optionsLastCycle.end())
      d_optionsLastCycle.erase(it);

    // Run it
    writer.String("run").StartObject();
    vector<shared_ptr<Option>> subOptions = option->runPolicy(writer);
    writer.EndObject();

    // Recurse through any sub-options
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

    shared_ptr<FSMOption> fsmOption = std::dynamic_pointer_cast<FSMOption,Option>(option);
    if (fsmOption)
      fsmStates.emplace_back(fsmOption->getId(), fsmOption->getCurrentState()->name);
  };

  runOption(d_top);

  // NOTE this is a bit weird/wasteful... we should generate the Document
  //      above and pass it instead of creating the string then parsing it...
  auto doc = make_unique<Document>();
  doc->Parse<0,UTF8<>>(buffer.GetString());

  State::make<OptionTreeState>(ranOptions, std::move(doc), fsmStates);

  // Reset options that ran in the last cycle but not in this one
  for (auto const& o : d_optionsLastCycle)
    o->reset();

  d_optionsLastCycle.clear();
  d_optionsLastCycle.insert(ranOptions.begin(), ranOptions.end());
}
