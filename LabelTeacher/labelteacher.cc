#include "labelteacher.hh"

#include "../StateObject/LabelTeacherState/labelteacherstate.hh"
#include "../State/state.hh"

using namespace bold;
using namespace std;

LabelTeacher::LabelTeacher(std::vector<std::shared_ptr<PixelLabel>> labels)
  : d_labels{labels},
  d_yuvTrainImage{},
  d_seedPoint{0,0},
  d_snapshotRequested{false},
  d_labelRequested{false},
  d_fixedRange{false}
{
  Config::getSetting<int>("label-teacher.max-flood-diff")->track([this](int val) {
      d_maxFloodDiff = val;
      log::info("HistogramLabelTeacherBase") << "Setting maxFloodDiff: " << d_maxFloodDiff;
    });

  Config::getSetting<double>("label-teacher.sigma-range")->track([this](int val) {
      d_sigmaRange = val;
      log::info("HistogramLabelTeacherBase") << "Setting sigmaRange: " << d_sigmaRange;
    });

  d_useRange = Config::getSetting<UseRange>("label-teacher.use-range");
  d_trainMode = Config::getSetting<TrainMode>("label-teacher.train-mode");

  std::map<int, std::string> enumOptions;

  for (unsigned i = 0; i < labels.size(); ++i)
    enumOptions[i] = labels[i]->getName();
  auto setting = new EnumSetting("label-teacher.label-to-train", enumOptions, false, "Label to train");
  setting->changed.connect([this](int value) {
      d_labelToTrain = value;
    });
  setting->setValue(0);

  Config::addSetting(setting);

  Config::getSetting<bool>("label-teacher.fixed-range")->track([this](bool val) {
      d_fixedRange = val;
    });

  Config::addAction("label-teacher.snap-train-image", "Snap Image", [this]()
                    {
                      if (d_yuvTrainImage.rows == 0)
                        d_snapshotRequested = true;
                      else
                        d_yuvTrainImage = cv::Mat{0,0,CV_8UC3};
                    });

  Config::addAction("label-teacher.set-seed-point", "Set Seed Point", [this](rapidjson::Value* val) {
      if (d_yuvTrainImage.empty())
        return;

      auto xMember = val->FindMember("x");
      auto yMember = val->FindMember("y");
      if (xMember == val->MemberEnd() || !xMember->value.IsInt() ||
          yMember == val->MemberEnd() || !yMember->value.IsInt())
      {
        log::error() << "Unable to parse x and y integral values from action JSON";
      }
      int x = xMember->value.GetInt(),
        y = yMember->value.GetInt();
      log::info("HistogramLabelTeacherBase") << "Setting seed point: " << x << " " << y;
      d_seedPoint = Eigen::Vector2i{x, y};

      d_mask = floodFill();
      updateState(d_mask);
    });

  Config::addAction("label-teacher.train", "Train", [this]() {
      train(d_labelToTrain, d_mask);
    });

  Config::addAction("label-teacher.label", "Label", [this]() {
      d_labelRequested = !d_labelRequested;
    });
}

std::vector<std::shared_ptr<PixelLabel>> LabelTeacher::getLabels() const
{
  return d_labels;
}

void LabelTeacher::setYUVTrainImage(cv::Mat yuvTrainImage)
{
  d_yuvTrainImage = yuvTrainImage;
  d_snapshotRequested = false;
}

void LabelTeacher::setSeedPoint(Eigen::Vector2i point)
{
  d_seedPoint = point;
}

cv::Mat LabelTeacher::floodFill() const
{
  cv::Mat mask = cv::Mat::zeros(d_yuvTrainImage.rows + 2, d_yuvTrainImage.cols + 2, CV_8UC1);

  auto mfd = cv::Scalar(d_maxFloodDiff, d_maxFloodDiff, d_maxFloodDiff);
  cv::floodFill(d_yuvTrainImage, mask,
                cv::Point{d_seedPoint.x(), d_seedPoint.y()}, cv::Scalar(255), 0,
                mfd, mfd, 4 | (d_fixedRange ? cv::FLOODFILL_FIXED_RANGE : 0) | cv::FLOODFILL_MASK_ONLY | (255 << 8));

  return cv::Mat{mask, cv::Rect(1, 1, d_yuvTrainImage.cols, d_yuvTrainImage.rows)};
    
  return mask;
}

void LabelTeacher::updateState(cv::Mat const& mask) const
{
  ASSERT(mask.type() == CV_8UC1);
  ASSERT(d_yuvTrainImage.cols == mask.cols && d_yuvTrainImage.rows == mask.rows);

  auto samples = getSamples(mask);
  auto range = determineRange(samples);
  auto distribution = determineDistribution(samples);

  State::make<LabelTeacherState>(range, distribution);
}

vector<Colour::hsv> LabelTeacher::getSamples(cv::Mat const& mask) const
{
  vector<Colour::hsv> samples;
  for (int i = 0; i < mask.rows; ++i)
  {
    uint8_t const* trainImageRow = d_yuvTrainImage.ptr<uint8_t>(i);
    uint8_t const* maskRow = mask.ptr<uint8_t>(i);

    for (int j = 0; j < mask.cols; ++j)
      if (maskRow[j] != 0)
      {
        auto yuv = Colour::YCbCr{trainImageRow[j * 3 + 0], trainImageRow[j * 3 + 1], trainImageRow[j * 3 + 2]};
        auto bgr = yuv.toBgrInt();
        auto hsv = bgr2hsv(bgr);

        samples.push_back(hsv);
      }
  }
  return samples;
}

Colour::hsvRange LabelTeacher::determineRange(const std::vector<Colour::hsv> &samples)
{
  auto range = Colour::hsvRange{samples[0].h, samples[0].h,
                                samples[0].s, samples[0].s,
                                samples[0].v, samples[0].v};
  for (auto const& hsv : samples)
  {
    range.hMin = min(range.hMin, hsv.h);
    range.hMax = max(range.hMax, hsv.h);
    range.sMin = min(range.sMin, hsv.s);
    range.sMax = max(range.sMax, hsv.s);
    range.vMin = min(range.vMin, hsv.v);
    range.vMax = max(range.vMax, hsv.v);
  }
  return range;
}

pair<Colour::hsv, Colour::hsv> LabelTeacher::determineDistribution(const std::vector<Colour::hsv> &samples)
{
  auto radians = std::vector<double>(samples.size());
  std::transform(std::begin(samples), std::end(samples), 
                 std::begin(radians),
                 [](Colour::hsv const& hsv) { return double(hsv.h) / 127.5 * M_PI; });
  
  double hMean = Math::angularMean(radians);
  double hSigma = 0;
  double sMean = 0;
  double sSigma = 0;
  double vMean = 0;
  double vSigma = 0;
  for (unsigned i = 0; i < samples.size(); ++i)
  {
    auto const& hsv = samples[i];
    double hDiff = Math::shortestAngleDiffRads(radians[i], hMean);
    hSigma += hDiff * hDiff;

    sMean += hsv.s;
    sSigma += double(hsv.s) * hsv.s;
    
    vMean += hsv.v;
    vSigma += double(hsv.v) * hsv.v;
  }
  hSigma = sqrt(hSigma / samples.size());
  hMean *= 127.5 / M_PI;
  hSigma *= 127.5 / M_PI;

  sMean /= samples.size();
  sSigma = sqrt(sSigma / samples.size() - sMean * sMean);

  vMean /= samples.size();
  vSigma = sqrt(vSigma / samples.size() - vMean * vMean);

  return std::make_pair(Colour::hsv(hMean, sMean, vMean), Colour::hsv(hSigma, sSigma, vSigma));
}

void LabelTeacher::train(unsigned labelIdx, cv::Mat const& mask)
{
  ASSERT(mask.type() == CV_8UC1);
  ASSERT(d_yuvTrainImage.cols == mask.cols && d_yuvTrainImage.rows == mask.rows);

  auto labelToTrain = d_labels[labelIdx];
  bool reset = d_trainMode->getValue() == TrainMode::Replace;

  auto samples = getSamples(d_mask);
  auto dist = determineDistribution(samples);

  Eigen::Vector3i hsvMin, hsvMax;

  hsvMin(0) = dist.first.h - d_sigmaRange * dist.second.h;
  hsvMax(0) = dist.first.h + d_sigmaRange * dist.second.h;
  hsvMin(1) = dist.first.s - d_sigmaRange * dist.second.s;
  hsvMax(1) = dist.first.s + d_sigmaRange * dist.second.s;
  hsvMin(2) = dist.first.v - d_sigmaRange * dist.second.v;
  hsvMax(2) = dist.first.v + d_sigmaRange * dist.second.v;

  if (hsvMin(0) < 0)
    hsvMin(0) += 256;
  if (hsvMax(0) > 255)
    hsvMax(0) -= 255;
  hsvMin(1) = max(hsvMin(1), 0);
  hsvMax(1) = min(hsvMax(1), 255);
  hsvMin(2) = max(hsvMin(2), 0);
  hsvMax(2) = min(hsvMax(2), 255);

  auto distRange = Colour::hsvRange(hsvMin(0), hsvMax(0), hsvMin(1), hsvMax(1), hsvMin(2), hsvMax(2));

  for (int i = 0; i < mask.rows; ++i)
  {
    uint8_t const* trainImageRow = d_yuvTrainImage.ptr<uint8_t>(i);
    uint8_t const* maskRow = mask.ptr<uint8_t>(i);

    for (int j = 0; j < mask.cols; ++j)
      if (maskRow[j] != 0)
      {
        auto yuv = Colour::YCbCr{trainImageRow[j * 3 + 0], trainImageRow[j * 3 + 1], trainImageRow[j * 3 + 2]};
        auto bgr = yuv.toBgrInt();
        auto hsv = bgr2hsv(bgr);

        if (d_useRange->getValue() == UseRange::XSigmas)
          if (!distRange.contains(hsv))
            continue;

        if (reset)
        {
          auto resetRange = Colour::hsvRange(hsv.h, hsv.h, hsv.s, hsv.s, hsv.v, hsv.v);
          labelToTrain->setHSVRange(resetRange);
          reset = false;
        }
        labelToTrain->addSample(hsv);
      }
  }

  // TODO: breaks when we use Histo labels
  for (auto label : d_labels)
  {
    auto setting = Config::getSetting<Colour::hsvRange>(std::string("vision.pixel-labels.") + label->getName());
    auto rangeLabel = std::static_pointer_cast<RangePixelLabel>(label);
    setting->setValue(rangeLabel->getHSVRange());
  }

}

cv::Mat LabelTeacher::label(unsigned labelIdx) const
{
  auto labelImg = cv::Mat{d_yuvTrainImage.rows, d_yuvTrainImage.cols, CV_8UC1};
  for (int i = 0; i < labelImg.rows; ++i)
  {
    uint8_t const* trainImageRow = d_yuvTrainImage.ptr<uint8_t>(i);
    uint8_t* labelRow = labelImg.ptr<uint8_t>(i);

    for (int j = 0; j < labelImg.cols; ++j)
    {
      auto yuv = Colour::YCbCr{trainImageRow[j * 3 + 0], trainImageRow[j * 3 + 1], trainImageRow[j * 3 + 2]};
      auto bgr = yuv.toBgrInt();
      auto hsv = bgr2hsv(bgr);
      labelRow[j] = d_labels[labelIdx]->labelProb(hsv) * 255;
    }
  }

  cv::normalize(labelImg, labelImg, 0, 255, cv::NORM_MINMAX);
  cv::Mat zero = cv::Mat::zeros(labelImg.rows, labelImg.cols, CV_8UC1);
  cv::Mat _colourLabelImage;
  cv::merge(std::vector<cv::Mat>{zero, labelImg, zero}, _colourLabelImage);
  return _colourLabelImage;
}
