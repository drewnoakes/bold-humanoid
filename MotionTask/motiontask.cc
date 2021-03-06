#include "motiontask.hh"

using namespace bold;
using namespace std;

std::string bold::getMotionRequestStatusName(MotionRequestStatus status)
{
  switch (status)
  {
    case MotionRequestStatus::Pending:   return "Pending";
    case MotionRequestStatus::Selected:  return "Selected";
    case MotionRequestStatus::Completed: return "Completed";
    case MotionRequestStatus::Ignored:   return "Ignored";
    default: return "Unknown";
  }
}

std::ostream& operator<<(std::ostream& stream, MotionRequestStatus status)
{
  return stream << bold::getMotionRequestStatusName(status);
}

std::string bold::getMotionTaskStatusName(MotionTaskStatus status)
{
  switch (status)
  {
    case MotionTaskStatus::Pending:   return "Pending";
    case MotionTaskStatus::Selected:  return "Selected";
    case MotionTaskStatus::Completed: return "Completed";
    case MotionTaskStatus::Ignored:   return "Ignored";
    default: return "Unknown";
  }
}

std::ostream& operator<<(std::ostream& stream, MotionTaskStatus status)
{
  return stream << bold::getMotionTaskStatusName(status);
}

std::string bold::getPriorityName(Priority priority)
{
  switch (priority)
  {
    case Priority::High:   return "High";
    case Priority::Normal: return "Normal";
    case Priority::Low:    return "Low";
    case Priority::None:   return "None";
    default: return "Unknown";
  }
}

std::ostream& operator<<(std::ostream& stream, Priority priority)
{
  return stream << bold::getPriorityName(priority);
}

////////////////////////////////////////////////////////////////////////////////

JointSelection::JointSelection(bool head, bool arms, bool legs)
: d_head(head),
  d_arms(arms),
  d_legs(legs)
{
  bool set = arms;
  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
  {
    if (jointId == (int)JointId::LEGS_START)
      set = legs;
    else if (jointId == (int)JointId::HEAD_START)
      set = head;

    d_set[jointId] = set;
  }
}

////////////////////////////////////////////////////////////////////////////////

void MotionRequest::setSectionTask(SectionId section, std::shared_ptr<MotionTask> const& task)
{
  switch (section)
  {
    case SectionId::Head: d_headTask = task; break;
    case SectionId::Arms: d_armsTask = task; break;
    case SectionId::Legs: d_legsTask = task; break;
    default:
      log::error("MotionRequest::setSectionTask") << "Invalid SectionId value: " << static_cast<int>(section);
      throw std::runtime_error("Invalid SectionId value");
  }
}

MotionRequestStatus MotionRequest::getStatus() const
{
  // Aggregate the state of constituent MotionTask instances.
  vector<shared_ptr<MotionTask const>> tasks = { d_headTask, d_armsTask, d_legsTask };

  int countNotNull = 0;
  int countIgnored = 0;
  int countCompleted = 0;

  for (auto const& task : tasks)
  {
    if (task == nullptr)
      continue;

    countNotNull++;

    switch (task->getStatus())
    {
      case MotionTaskStatus::Pending:
        // If any task is pending, then the request is pending
        return MotionRequestStatus::Pending;

      case MotionTaskStatus::Selected:
        // If anything is being run, then we're selected
        return MotionRequestStatus::Selected;

      case MotionTaskStatus::Completed: countCompleted++; break;
      case MotionTaskStatus::Ignored:   countIgnored++;   break;
    }
  }

  if (countNotNull == 0)
    return MotionRequestStatus::Ignored;

  // If all tasks were ignored, then the entire request was ignored
  if (countIgnored == countNotNull)
    return MotionRequestStatus::Ignored;

  // Otherwise, we must be complete
  return MotionRequestStatus::Completed;
}

bool MotionRequest::hasCompleted() const
{
  auto status = getStatus();
  return status == MotionRequestStatus::Completed || status == MotionRequestStatus::Ignored;
}
