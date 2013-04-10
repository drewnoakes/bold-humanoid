#ifndef BOLD_OPTION_HH
#define BOLD_OPTION_HH

#include <vector>
#include <memory>
#include <string>

namespace bold
{
  class Option;
  typedef std::shared_ptr<Option> OptionPtr;

  /** Option
   *
   * An option is an abstract description of an action, that may be
   * temporally extended and semi-markovian. This means it may be
   * active for any length of time, and its policy may rely not only
   * on the current state, but on any part of history. An option is
   * formed by a three-tuple @f$<I,\beta,\mu>@f, where:
   *
   * * @f$I@f$ is the set of states in which this option can be active
   * * @f$\beta@f$ gives the probaility of this option terminating now
   * * @f$\mu@f$ is a policy over (sub)-options that selects which
   *   action to take at which time
   *
   */
  class Option {
  public:
    Option(std::string const& id) : d_id(id) {}
    virtual ~Option() {}

    /** Get this option's ID
     */
    std::string getID() const { return d_id; }

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

    /** Select this option to be run
     *
     * If this option returns a null pointer, this indicates that it
     * is a primitive option.
     *
     * @returns the sub-option selected by the policy of this option;
     * Default: null-pointer
     */
    virtual OptionPtr runPolicy() { return OptionPtr(0); }

  private:

    std::string d_id;
  };

}

#endif


