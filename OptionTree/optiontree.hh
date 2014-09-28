#pragma once

#include "../Option/option.hh"

#include <map>
#include <set>

namespace bold
{
  class FSMOption;

  /**
   * The option tree models the complete behaviour of the agent.
   * It is executed each think cycle after sensor data has been processed.
   * Consequences of its running include motion, network messages and internal state updates.
   */
  class OptionTree
  {
  public:
    OptionTree(std::shared_ptr<Option> root);

    /**
     * Executes the option tree, from the root down.
     */
    void run();

    /**
     * Registers an FSM option with the tree such that it may be debugged in Round Table.
     */
    void registerFsm(std::shared_ptr<FSMOption> fsm);

    /**
     * Gets all previously registered FSM options.
     */
    std::vector<std::shared_ptr<FSMOption>> getFSMs() const;

  private:
    std::map<std::string, std::shared_ptr<FSMOption>> d_fsmOptions;
    std::set<std::shared_ptr<Option>> d_optionsLastCycle;
    const std::shared_ptr<Option> d_root;
  };
}
