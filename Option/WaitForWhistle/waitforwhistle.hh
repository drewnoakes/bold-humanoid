#pragma once

#include "../option.hh"
#include "../../WhistleListener/whistlelistener.hh"

namespace bold
{
  template<typename> class Setting;

  class WaitForWhistle : public Option
  {
  public:
    WaitForWhistle(std::string id);

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    double hasTerminated() override;

    void reset() override;

  private:
    std::unique_ptr<WhistleListener> d_listener;
    Setting<WindowFunctionType>* d_windowType;
    bool d_initialised;
    bool d_terminated;
  };
}
