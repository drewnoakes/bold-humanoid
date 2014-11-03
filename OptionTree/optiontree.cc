#include "optiontree.hh"

#include "../Option/FSMOption/fsmoption.hh"
#include "../State/state.hh"
#include "../StateObject/OptionTreeState/optiontreestate.hh"
#include "../util/assert.hh"
#include "../util/log.hh"

#include <functional>
#include <rapidjson/document.h>
#include <sstream>
#include <fstream>

using namespace bold;
using namespace rapidjson;
using namespace std;

OptionTree::OptionTree(std::shared_ptr<Option> root)
  : d_root(root)
{}

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
    log::trace("OptionTree::run") << "Running " << option->getTypeName() << " option: " << option->getId();

    writer.StartObject();

    writer.String("id");
    writer.String(option->getId().c_str());
    writer.String("type");
    writer.String(option->getTypeName().c_str());

    // Remove options that run from the set of last cycle
    auto it = d_optionsLastCycle.find(option);
    if (it != d_optionsLastCycle.end())
      d_optionsLastCycle.erase(it);

    // Run it
    writer.String("run");
    writer.StartObject();
    vector<shared_ptr<Option>> subOptions = option->runPolicy(writer);
    writer.EndObject();

    // Recur through any sub-options
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

  runOption(d_root);

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

void OptionTree::registerFsm(std::shared_ptr<FSMOption> fsm)
{
  if (d_fsmOptions.count(fsm->getId()))
  {
    log::error("OptionTree::registerFsm") << "Attempted to register FSM with id='" << fsm->getId() << "' more than once";
    throw runtime_error("Attempt to register FSM ID more than once");
  }

  d_fsmOptions[fsm->getId()] = fsm;

  // Validate the FSM
  if (!fsm->getStartState())
  {
    log::error("OptionTree::registerFsm") << "Attempt to add an FSMOption with ID '" << fsm->getId() << "' which has no start state";
    throw std::runtime_error("Attempt to add an FSMOption which has no start state");
  }

//  // Write out its digraph to disk
//  std::stringstream fileName;
//  fileName << fsm->getId() << ".dot";
//  std::ofstream winOut(fileName.str());
//  winOut << fsm->toDot();
}

vector<shared_ptr<FSMOption>> OptionTree::getFSMs() const
{
  vector<shared_ptr<FSMOption>> fsmOptions;
  for (auto const& fsm : d_fsmOptions)
    fsmOptions.push_back(fsm.second);
  return fsmOptions;
}