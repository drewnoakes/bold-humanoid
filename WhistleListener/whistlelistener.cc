#include "whistlelistener.hh"

#include "../util/log.hh"
#include "../Config/config.hh"
#include "../State/state.hh"
#include "../StateObject/AudioPowerSpectrumState/audiopowerspectrumstate.hh"

using namespace bold;
using namespace std;

int doSnd(function<int()> operation, const char* description)
{
  int err = operation();

  if (err < 0)
  {
    log::warning("WhistleListener") << "Cannot " << description << " (" << snd_strerror(err) << ")";
    throw runtime_error("Error calling ALSA API");
  }

  return err;
}

void logPcmInfo(snd_pcm_t* handle, snd_pcm_hw_params_t *params)
{
  unsigned int val, val2;
  int dir;
  snd_pcm_uframes_t frames;
  snd_pcm_format_t fmt;

  /* Display information about the PCM interface */

  printf("PCM handle name = '%s'\n", snd_pcm_name(handle));
  printf("PCM state = %s\n", snd_pcm_state_name(snd_pcm_state(handle)));

  snd_pcm_access_t access;
  snd_pcm_hw_params_get_access(params, &access);
  printf("access type = %s\n", snd_pcm_access_name(access));

  snd_pcm_hw_params_get_format(params, &fmt);
  printf("format = '%s' (%s)\n",
    snd_pcm_format_name(fmt),
    snd_pcm_format_description(fmt));

  snd_pcm_subformat_t subformat;
  snd_pcm_hw_params_get_subformat(params, &subformat);
  printf("subformat = '%s' (%s)\n",
    snd_pcm_subformat_name(subformat),
    snd_pcm_subformat_description(subformat));

  snd_pcm_hw_params_get_channels(params, &val);
  printf("channels = %d\n", val);

  snd_pcm_hw_params_get_rate(params, &val, &dir);
  printf("rate = %d bps\n", val);

  snd_pcm_hw_params_get_period_time(params, &val, &dir);
  printf("period time = %d us\n", val);

  snd_pcm_hw_params_get_period_size(params, &frames, &dir);
  printf("period size = %d frames\n", (int)frames);

  snd_pcm_hw_params_get_buffer_time(params, &val, &dir);
  printf("buffer time = %d us\n", val);

  snd_pcm_hw_params_get_buffer_size(params, &frames);
  printf("buffer size = %d frames\n", (int)frames);

  snd_pcm_hw_params_get_periods(params, &val, &dir);
  printf("periods per buffer = %d frames\n", val);

  snd_pcm_hw_params_get_rate_numden(params, &val, &val2);
  printf("exact rate = %d/%d bps\n", val, val2);

  printf("significant bits = %d\n", snd_pcm_hw_params_get_sbits(params));
  printf("is batch = %d\n", snd_pcm_hw_params_is_batch(params));
  printf("is block transfer = %d\n", snd_pcm_hw_params_is_block_transfer(params));
  printf("is double = %d\n", snd_pcm_hw_params_is_double(params));
  printf("is half duplex = %d\n", snd_pcm_hw_params_is_half_duplex(params));
  printf("is joint duplex = %d\n", snd_pcm_hw_params_is_joint_duplex(params));
  printf("can overrange = %d\n", snd_pcm_hw_params_can_overrange(params));
  printf("can mmap = %d\n", snd_pcm_hw_params_can_mmap_sample_resolution(params));
  printf("can pause = %d\n", snd_pcm_hw_params_can_pause(params));
  printf("can resume = %d\n", snd_pcm_hw_params_can_resume(params));
  printf("can sync start = %d\n", snd_pcm_hw_params_can_sync_start(params));
}

WhistleListener::WhistleListener()
: WhistleListener(
  Config::getValue<string>("hardware.microphone-name"),
  (uint)Config::getValue<int>("whistle-detection.sample-rate-hz"),
  (uint)Config::getValue<int>("whistle-detection.sample-count"),
  (ushort)Config::getValue<int>("whistle-detection.fast-smoothing-length"),
  (ushort)Config::getValue<int>("whistle-detection.slow-smoothing-length"))
{}

WhistleListener::WhistleListener(string deviceName, uint sampleRate, uint sampleCount, ushort slowSmoothingLength, ushort fastSmoothingLength)
: d_deviceName(deviceName),
  d_sampleRate(sampleRate),
  d_sampleCount(sampleCount),
  d_slowSmoothers(),
  d_fastSmoothers(),
  d_window(sampleCount),
  d_pcm(nullptr)
{
  setWindowFunction(WindowFunctionType::Rectangular);

  for (uint i = 0; i < (sampleCount / 2) + 1; i++)
  {
    d_slowSmoothers.emplace_back(slowSmoothingLength);
    d_fastSmoothers.emplace_back(fastSmoothingLength);
  }

  d_samples = fftwf_alloc_real(d_sampleCount);
  d_frequencies = fftwf_alloc_complex(d_sampleCount);
  d_fftwPlan = fftwf_plan_dft_r2c_1d(d_sampleCount, d_samples, d_frequencies, FFTW_ESTIMATE);
}

WhistleListener::~WhistleListener()
{
  log::verbose("WhistleListener::~WhistleListener");

  if (d_pcm)
  {
    int res = snd_pcm_close(d_pcm);
    if (res < 0)
      log::warning("WhistleListener::~WhistleListener") << "Error closing PCM: " << snd_strerror(res);
  }

  fftwf_destroy_plan(d_fftwPlan);
  fftwf_free(d_samples);
  fftwf_free(d_frequencies);
}

bool WhistleListener::initialise()
{
  log::verbose("WhistleListener::initialise");

  snd_pcm_hw_params_t *hwParams = nullptr;

  try
  {
    // Open the PCM for audio capture
    doSnd([&] { return snd_pcm_open(&d_pcm, d_deviceName.c_str(), SND_PCM_STREAM_CAPTURE, 0); }, "open audio device");
    // Create the hardware parameters struct memory
    doSnd([&] { return snd_pcm_hw_params_malloc(&hwParams); }, "allocate hardware parameter structure");
    // Read the current parameters
    doSnd([&] { return snd_pcm_hw_params_any(d_pcm, hwParams); }, "initialize hardware parameter structure");
    // Set interleaved access
    doSnd([&] { return snd_pcm_hw_params_set_access(d_pcm, hwParams, SND_PCM_ACCESS_RW_INTERLEAVED); }, "set access type");
    // Set 32-bit floating point (little endian) format
    doSnd([&] { return snd_pcm_hw_params_set_format(d_pcm, hwParams, SND_PCM_FORMAT_FLOAT_LE); }, "set sample format");
    // Set the sample rate
    doSnd([&] { return snd_pcm_hw_params_set_rate(d_pcm, hwParams, d_sampleRate, 0); }, "set sample rate");
    // Set single channel (mono)
    doSnd([&] { return snd_pcm_hw_params_set_channels(d_pcm, hwParams, 1); }, "set channel count");
    // Write hardware params to PCM
    doSnd([&] { return snd_pcm_hw_params(d_pcm, hwParams); }, "set parameters");

//    logPcmInfo(d_pcm, hwParams);

    snd_pcm_hw_params_free(hwParams);

    // Open the audio device
    doSnd([&] { return snd_pcm_prepare(d_pcm); }, "prepare pcm");
    doSnd([&] { return snd_pcm_start(d_pcm); }, "start pcm");
  }
  catch (runtime_error err)
  {
    snd_pcm_hw_params_free(hwParams);
    return false;
  }

  return true;
}

bool WhistleListener::step()
{
  ASSERT(d_pcm);

  while (snd_pcm_avail(d_pcm) >= (long)d_sampleCount)
  {
    // Get audio samples

    snd_pcm_sframes_t read = snd_pcm_readi(d_pcm, d_samples, d_sampleCount);

    if (read == -EPIPE)
    {
      log::warning("WhistleListener::step") << "An overrun occurred";
      return false;
    }

    if (read < 0)
    {
      log::error("WhistleListener::step") << "Error reading from buffer: " << snd_strerror((int) read);
      return false;
    }

    if (read != (long)d_sampleCount)
    {
      // not enough samples available
      log::error("WhistleListener::step") << "Too few samples: " << read << ", not " << d_sampleCount;
      return false;
    }

    static auto testFrequency = Config::getSetting<double>("whistle-detection.test-frequency-hz");
    if (testFrequency->getValue() != 0)
    {
      // Replace sampled audio with a pure sine wave (for testing)
      const double period = d_sampleRate / testFrequency->getValue();
      for (uint i = 0; i < d_sampleCount; i++)
        d_samples[i] = (float)sin(2 * M_PI * (i / period));
    }

    // Apply window function

    if (d_windowType != WindowFunctionType::Rectangular)
    {
      // TODO do we have to normalise when using a window? http://stackoverflow.com/a/8541731/24874
      for (uint i = 0; i < d_sampleCount; i++)
        d_samples[i] *= d_window[i];
    }

    // Perform FFT

    fftwf_execute(d_fftwPlan);

    vector<float> dbs(d_sampleCount / 2 + 1);

    // TODO group bins into bands (min/max/avg)

    for (uint i = 0; i <= d_sampleCount / 2; i++)
    {
      float re = d_frequencies[i][0];
      float im = d_frequencies[i][1];
      float mag = sqrt(re * re + im * im) / (d_sampleCount / (float)2.0);
      // TODO avoid sqrt here as log(a^b)==b*log(a) (what about above normalisation term?)
      float db = 20 * std::log10(mag);
      dbs[i] = d_slowSmoothers[i].next(db) - d_fastSmoothers[i].next(db);
    }

    auto spectrum = State::make<AudioPowerSpectrumState>(d_sampleRate/2, dbs);

    // Analyse detected frequencies and determine if a whistle is blowing
    static auto minFreqHz = Config::getSetting<double>("whistle-detection.min-freq-hz");
    static auto maxFreqHz = Config::getSetting<double>("whistle-detection.max-freq-hz");
    static auto widthFreqHz = Config::getSetting<double>("whistle-detection.width-freq-hz");
    static auto thresholdDbRatio = Config::getSetting<double>("whistle-detection.threshold-db-ratio");
    static auto thresholdDb = Config::getSetting<double>("whistle-detection.threshold-db");

    // Find the peak level between the boundary frequencies
    uint lowerIndex = spectrum->getIndexForFreqHz(minFreqHz->getValue());
    uint upperIndex = spectrum->getIndexForFreqHz(maxFreqHz->getValue());
//    cout << "minFreqHz=" << minFreqHz->getValue() << " maxFreqHz=" << maxFreqHz->getValue() << endl;
//    cout << "lowerIndex=" << lowerIndex << " upperIndex=" << upperIndex << endl;
    double peakDb = -numeric_limits<double>::max();
    uint peakIndex = 0;
    for (uint i = lowerIndex; i < upperIndex; i++)
    {
      float db = spectrum->getDecibelsByIndex(i);
//      cout << "i=" << i << " db=" << db << endl;
      if (db > peakDb)
      {
//        cout << "...greater" << endl;
        peakDb = db;
        peakIndex = i;
      }
    }

    // Sum up levels around the peak
    uint widthAsIndex = spectrum->getIndexForFreqHz(widthFreqHz->getValue()) / 2;
    lowerIndex = max(peakIndex - widthAsIndex, (uint)0);
    upperIndex = min(peakIndex + widthAsIndex, spectrum->getMaxIndex());
//    cout << "widthAsIndex=" << widthAsIndex << " lowerIndex=" << lowerIndex << " upperIndex=" << upperIndex << endl;
    float sumDb = 0.0;
    for (uint i = lowerIndex; i < upperIndex; i++)
      sumDb += max((float)0.0, spectrum->getDecibelsByIndex(i));

    // Is the peak significantly distant from the average level?
    double averageDb = sumDb / (upperIndex - lowerIndex);
    cout
      << "dbRatio=" << (averageDb/peakDb)
      << " peakDb=" << peakDb
      << " peakIndex=" << peakIndex
      << " sumDb=" << sumDb
      << " averageDb=" << averageDb
//      << " thresholdDbRatio=" << thresholdDbRatio->getValue()
      << endl;
    if (sumDb != 0 && peakDb > thresholdDb->getValue() && averageDb/peakDb < thresholdDbRatio->getValue())
      return true;
  }

  return false;
}

void WhistleListener::setWindowFunction(WindowFunctionType type)
{
  vector<double> window(d_sampleCount);
  WindowFunction::create(type, window);

  for (uint i = 0; i < d_sampleCount; i++)
    d_window[i] = (float)window[i];
}
