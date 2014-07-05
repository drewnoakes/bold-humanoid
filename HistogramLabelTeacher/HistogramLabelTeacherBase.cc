#include "histogramlabelteacher.ih"

HistogramLabelTeacherBase::HistogramLabelTeacherBase(std::vector<std::string> const& names)
  : d_yuvTrainImage{},
  d_seedPoint{0,0},
  d_snapshotRequested{false},
  d_labelRequested{false},
  d_fixedRange{false}
{
  Config::getSetting<int>("histogram-label-teacher.max-flood-diff")->track([this](int val) {
      d_maxFloodDiff = val;
      log::info("HistogramLabelTeacherBase") << "Setting maxFloodDiff: " << d_maxFloodDiff;
    });

  map<int, string> enumOptions;
  for (unsigned i = 0; i < names.size(); ++i)
    enumOptions[i] = names[i];

  auto setting = new EnumSetting("histogram-label-teacher.label-to-train", enumOptions, false, "Label to train");
  setting->changed.connect([names,this](int value) {
      d_labelToTrain = value;
    });
  setting->setValue(0);

  Config::addSetting(setting);

  Config::getSetting<bool>("histogram-label-teacher.fixed-range")->track([this](bool val) {
      d_fixedRange = val;
    });

  Config::addAction("histogram-label-teacher.snap-train-image", "Snap Image", [this]()
                    {
                      if (d_yuvTrainImage.rows == 0)
                        d_snapshotRequested = true;
                      else
                        d_yuvTrainImage = cv::Mat{0,0,CV_8UC3};
                    });

  Config::addAction("histogram-label-teacher.set-seed-point", "Set Seed Point", [this](rapidjson::Value* val) {
      int x, y;
      val->TryGetIntValue("x", &x);
      val->TryGetIntValue("y", &y);
      log::info("HistogramLabelTeacherBase") << "Setting seed point: " << x << " " << y;
      d_seedPoint = Vector2i{x, y};

      d_mask = floodFill();
    });

  Config::addAction("histogram-label-teacher.train", "Train", [this]() {
      train(d_labelToTrain, d_mask);
    });

  Config::addAction("histogram-label-teacher.label", "Label", [this]() {
      d_labelRequested = !d_labelRequested;
    });
}
