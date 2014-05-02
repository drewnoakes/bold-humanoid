#pragma once

#include <memory>

#include "../JointId/jointid.hh"
#include "../util/assert.hh"
#include "../util/log.hh"

namespace bold
{
  class MotionModule;
  class MotionTask;

  typedef unsigned char uchar;

  /** The priority of a motion task.
   *
   * Used in arbitrating which motion module gains control of a body section.
   */
  enum class Priority
  {
    High   = 3,
    Normal = 2,
    Low    = 1,
    None   = 0
  };

  enum class SectionId
  {
    Head,
    Arms,
    Legs
  };

  //////////////////////////////////////////////////////////////////////////////

  /** Models a set of joints.
   */
  class JointSelection
  {
  public:
    JointSelection(bool head, bool arms, bool legs);

    bool const& operator[] (uchar jointId) const { return d_set[jointId]; }

    bool hasHead() const { return d_head; }
    bool hasArms() const { return d_arms; }
    bool hasLegs() const { return d_legs; }

    static std::shared_ptr<JointSelection> const& all()         { static auto instance = std::make_shared<JointSelection>(true,  true,  true);  return instance; }
    static std::shared_ptr<JointSelection> const& head()        { static auto instance = std::make_shared<JointSelection>(true,  false, false); return instance; }
    static std::shared_ptr<JointSelection> const& arms()        { static auto instance = std::make_shared<JointSelection>(false, true,  false); return instance; }
    static std::shared_ptr<JointSelection> const& legs()        { static auto instance = std::make_shared<JointSelection>(false, false, true);  return instance; }
    static std::shared_ptr<JointSelection> const& headAndArms() { static auto instance = std::make_shared<JointSelection>(true,  true,  false); return instance; }
    static std::shared_ptr<JointSelection> const& headAndLegs() { static auto instance = std::make_shared<JointSelection>(true,  false, true);  return instance; }
    static std::shared_ptr<JointSelection> const& armsAndLegs() { static auto instance = std::make_shared<JointSelection>(false, true,  true);  return instance; }

  private:
    bool d_set[21];
    bool d_head;
    bool d_arms;
    bool d_legs;
  };

  //////////////////////////////////////////////////////////////////////////////

  enum class MotionRequestStatus
  {
    /// Request is queued in the scheduler but has not yet been evaluated for
    /// selection.
    Pending,

    /// Request was selected by the scheduler and is executing.
    Selected,

    /// Request was selected and has since run to completion, either by
    /// completing its single think cycle of control, or by being committed and
    /// signaling completion explicitly.
    Completed,

    /// Request did not succeed in being selected. Another request may have been
    /// committed, or had higher priority.
    Ignored
  };

  std::string getMotionRequestStatusName(MotionRequestStatus status);

  /** Models a request to control one or more body sections by a MotionModule.
   *
   * Within this request, different parameters may be selected per body section.
   */
  class MotionRequest
  {
    friend class MotionTaskScheduler;

  public:
    MotionRequestStatus getStatus() const;
    std::shared_ptr<MotionTask const> getHeadTask() const { return d_headTask; }
    std::shared_ptr<MotionTask const> getArmsTask() const { return d_armsTask; }
    std::shared_ptr<MotionTask const> getLegsTask() const { return d_legsTask; }

  protected:
    void setSectionTask(SectionId section, std::shared_ptr<MotionTask> const& task);

  private:
    std::shared_ptr<MotionTask const> d_headTask;
    std::shared_ptr<MotionTask const> d_armsTask;
    std::shared_ptr<MotionTask const> d_legsTask;
  };

  //////////////////////////////////////////////////////////////////////////////

  enum class MotionTaskStatus
  {
    Pending,
    Selected,
    Completed,
    Ignored
  };

  std::string getMotionTaskStatusName(MotionTaskStatus status);

  //////////////////////////////////////////////////////////////////////////////

  /** Represents a desire to control a body section using a particular motion module.
   *
   * May also request to maintain control of the body section until some later
   * point, known as commital.
   *
   * This internal class is instantiated by the MotionTaskScheduler and is not
   * intended for direct use.
   */
  class MotionTask
  {
  public:
    MotionTask(std::shared_ptr<MotionRequest> request, MotionModule* module, SectionId section, Priority priority, bool isCommitRequested)
    : d_request(request),
      d_module(module),
      d_section(section),
      d_priority(priority),
      d_isCommitRequested(isCommitRequested),
      d_isCommitted(false),
      d_status(MotionTaskStatus::Pending)
    {}

    MotionModule* getModule() const { return d_module; }
    SectionId getSection() const { return d_section; }
    Priority getPriority() const { return d_priority; }
    MotionTaskStatus getStatus() const { return d_status; }
    bool isCommitRequested() const { return d_isCommitRequested; }
    bool isCommitted() const { return d_isCommitted; }

    /// Called by the framework when a task that requests committal is started.
    void setCommitted() { ASSERT(d_isCommitRequested); d_isCommitted = true; }

    /// For testing purposes
    void clearCommitted() { d_isCommitted = false; }
    void setPriority(Priority priority) { d_priority = priority; }

    void setSelected()  { ASSERT(d_status == MotionTaskStatus::Pending  || d_status == MotionTaskStatus::Selected);  d_status = MotionTaskStatus::Selected; }
    void setIgnored()   { ASSERT(d_status == MotionTaskStatus::Pending  || d_status == MotionTaskStatus::Ignored);   d_status = MotionTaskStatus::Ignored; }
    void setCompleted() { ASSERT(d_status == MotionTaskStatus::Selected || d_status == MotionTaskStatus::Completed); d_status = MotionTaskStatus::Completed; }

  private:
    std::shared_ptr<MotionRequest> d_request;
    MotionModule* d_module;
    SectionId d_section;
    Priority d_priority;
    bool d_isCommitRequested;
    bool d_isCommitted;
    MotionTaskStatus d_status;
  };
}

std::ostream& operator<<(std::ostream& stream, bold::MotionRequestStatus status);
