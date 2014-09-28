#pragma once

#include <vector>
#include <memory>
#include <string>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace bold
{
  /** Option
   *
   * An option is an abstract description of an action that may be
   * temporally extended and semi-Markovian. This means it may be
   * active for any length of time, and its policy may rely not only
   * on the current state, but on any part of history. An option is
   * formed by a three-tuple @f$<I,\beta,\mu>@f, where:
   *
   * * @f$I@f$ is the set of states in which this option can be active
   * * @f$\beta@f$ gives the probability of this option terminating now
   * * @f$\mu@f$ is a policy over (sub)-options that selects which
   *   action to take at which time
   *
   */
  class Option
  {
  public:
    typedef std::vector<std::shared_ptr<Option>> OptionVector;

    Option(std::string const& id, std::string const& typeName)
    : d_id(id),
      d_typeName(typeName)
    {}

    virtual ~Option() = default;

    /** Get this option's ID
     */
    std::string getId() const { return d_id; }

    /** Get the type of this option, as a string
     */
    std::string getTypeName() const { return d_typeName; }

    /** Check whether this option is currently available
     *
     * @returns whether this option is available. Default: always true
     */
    virtual bool isAvailable() { return true; }

    /** Check the probability this option having terminated
     *
     * @return the probability that this option has reached its
     * goal. Default: always 1.0, i.e. single step action
     */
    virtual double hasTerminated() { return 1; }

    virtual void reset() {}

    /** Select this option to be run
     *
     * If this option returns an empty vector, this indicates that it
     * is a primitive option. Override this version to be able to send
     * JSON data to round table.
     *
     * @returns the sub-option selected by the policy of this option;
     * Default: empty vector
     */
    virtual OptionVector runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) { return runPolicy(); }

    /** Select this option to be run
     *
     * If this option returns an empty vector, this indicates that it
     * is a primitive option.
     *
     * @returns the sub-option selected by the policy of this option;
     * Default: empty vector
     */
    virtual OptionVector runPolicy() { return OptionVector{}; }

  private:
    std::string d_id;
    std::string d_typeName;
  };
}
