#include "cartoonpass.hh"

#include "../../ImageLabelData/imagelabeldata.hh"
#include "../../SequentialTimer/sequentialtimer.hh"

using namespace bold;

void CartoonPass::process(ImageLabelData const& labelData, SequentialTimer& timer)
{
  d_mat = cv::Scalar(0);
  timer.timeEvent("Clear");

  for (auto const& row : labelData)
  {
    uchar* ptr = d_mat.ptr<uchar>(row.imageY);
    auto dx = row.granularity.x();
    for (auto const& label : row)
    {
      // NOTE Have tested whether it was better to check if value is zero here, but it runs the
      //      pass 0.3ms slower on average with the if-check here, so skip it. The write pattern
      //      here is sequential anyway. If this becomes random access, it may help.
      *ptr = label;
      ptr += dx;
    }
  }
  timer.timeEvent("Process Rows");
}
