#pragma once

#include <alsa/asoundlib.h>
#include <fftw3.h>
#include <string>

#include "../stats/movingaverage.hh"
#include "../util/windowfunctions.hh"

namespace bold
{
  class WhistleListener
  {
  public:
    WhistleListener();

    WhistleListener(std::string deviceName, uint sampleRate, uint sampleCount, ushort slowSmoothingLength, ushort fastSmoothingLength);

    ~WhistleListener();

    bool initialise();

    bool step();

    void setWindowFunction(WindowFunctionType type);

  private:
    const std::string d_deviceName;
    uint d_sampleRate;
    uint d_sampleCount;
    std::vector<MovingAverage<float>> d_slowSmoothers;
    std::vector<MovingAverage<float>> d_fastSmoothers;
    std::vector<float> d_window;
    WindowFunctionType d_windowType;
    snd_pcm_t* d_pcm;
    float* d_samples;
    fftwf_complex* d_frequencies;
    fftwf_plan d_fftwPlan;
  };
}