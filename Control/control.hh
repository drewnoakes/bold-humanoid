#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace bold
{
  // TODO SETTINGS rename to 'Action'

  class Control
  {
  public:
    // TODO SETTINGS convert to constructors?

    static std::shared_ptr<Control> createAction(std::string name,
                                    std::function<void()> callback);

    static std::shared_ptr<Control> createAction(unsigned id,
                                    std::string name,
                                    std::function<void()> callback);

    Control()
    : d_name()
    {}

    /** Handles a request against this control. */
    void handleRequest() const;

    unsigned getId() const { return d_id; }
    std::string getName() const { return d_name; }

    friend std::ostream& operator<<(std::ostream& stream, Control const& control)
    {
      return stream << control.getName();
    }

  private:
    static unsigned s_nextControlId;

    unsigned d_id;
    std::string d_name;
    std::function<void()> d_callback;
  };
}
