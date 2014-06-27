#include "gtest/gtest.h"
#include "../HistogramLabelTeacher/histogramlabelteacher.hh"

#include <vector>

using namespace std;
using namespace bold;

TEST (HistogramLabelTeacherTests, init)
{
  auto names = vector<string>{string{"one"}, string{"two"}};
  HistogramLabelTeacher<6> teacher{names};

  auto labels = teacher.getLabels();
}
