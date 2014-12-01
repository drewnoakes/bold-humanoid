#pragma once

#include "../stateobject.hh"
#include "../../Option/option.hh"

#include <rapidjson/document.h>
#include <vector>
#include <memory>
#include <string>

namespace bold
{
  class FSMStateSnapshot
  {
  public:
    FSMStateSnapshot(std::string fsmName, std::string stateName)
      : d_fsmName(fsmName),
        d_stateName(stateName)
    {}

    std::string const& getFsmName() const { return d_fsmName; }
    std::string const& getStateName() const { return d_stateName; }

  private:
    std::string d_fsmName;
    std::string d_stateName;
  };

  class OptionTreeState : public StateObject
  {
  public:
    // TODO is OptionList backed by a mutable object? if so, we may end up sending incorrect information to clients from the data streamer loop
    OptionTreeState(std::vector<std::shared_ptr<Option>> const& ranOptions, std::unique_ptr<rapidjson::Document> optionJson, std::vector<FSMStateSnapshot> fsmStates)
    : d_ranOptions(ranOptions),
      d_optionJson(std::move(optionJson)),
      d_fsmStates(fsmStates)
    {}

    std::vector<std::shared_ptr<Option>> getRanOptions() const { return d_ranOptions; };
    std::vector<FSMStateSnapshot> getFSMStates() const { return d_fsmStates; };

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    std::vector<std::shared_ptr<Option>> d_ranOptions;
    std::unique_ptr<rapidjson::Document> d_optionJson;
    std::vector<FSMStateSnapshot> d_fsmStates;
  };

  template<typename TBuffer>
  inline void OptionTreeState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
  {
    writer.StartObject();
    {
      writer.String("ranoptions");
      writer.StartArray();
      for (auto const& option : d_ranOptions)
        writer.String(option->getId().c_str());
      writer.EndArray();

      writer.String("fsms");
      writer.StartArray();
      for (auto const& fsmState : d_fsmStates)
      {
        writer.StartObject();
        writer.String("fsm");
        writer.String(fsmState.getFsmName().c_str());
        writer.String("state");
        writer.String(fsmState.getStateName().c_str());
        writer.EndObject();
      }
      writer.EndArray();

      // Inject the option JSON document at this point in our output
      writer.String("path");
      d_optionJson->Accept(writer);
    }
    writer.EndObject();
  }
}
