#include <emscripten.h>

#include "BTT.h"
#include "Chromagram.h"
#include "ChordDetector.h"

#include <vector>
// #include <iostream>

void onset_detected_callback(void* SELF, unsigned long long sample_time);
void beat_detected_callback (void* SELF, unsigned long long sample_time);

class BTTObject {
public:
  BTT *btt;
  Chromagram *chromagram;
  double sampleTimestamp; 

  std::vector<unsigned long long> onsetTimestamps;
  std::vector<unsigned long long> beatTimestamps;
  
  BTTObject(double sampleRate = BTT_SUGGESTED_SAMPLE_RATE) {
    btt = btt_new(BTT_SUGGESTED_SPECTRAL_FLUX_STFT_LEN,
      BTT_SUGGESTED_SPECTRAL_FLUX_STFT_OVERLAP,
      BTT_SUGGESTED_OSS_FILTER_ORDER,
      BTT_SUGGESTED_OSS_LENGTH,
      BTT_SUGGESTED_ONSET_THRESHOLD_N,
      BTT_SUGGESTED_CBSS_LENGTH,
      sampleRate,
      BTT_DEFAULT_ANALYSIS_LATENCY_ONSET_ADJUSTMENT,
      BTT_DEFAULT_ANALYSIS_LATENCY_BEAT_ADJUSTMENT
    );
    btt_set_onset_tracking_callback(btt, onset_detected_callback, this);
    btt_set_beat_tracking_callback(btt, beat_detected_callback, this);
    btt_set_num_tempo_candidates(btt, 32); // BTT_DEFAULT_NUM_TEMPO_CANDIDATES

    constexpr int frameSize = 512; // changed dynamically
    chromagram = new Chromagram(frameSize, sampleRate);

    sampleTimestamp = 0;
  }
};

struct BTTProcessOutput {
  double sampleTimestamp;
  float bpm;
  unsigned int numOnsetTimestamps;
  unsigned long long *onsetTimestamps;
  unsigned int numBeatTimestamps;
  unsigned long long *beatTimestamps;

  int rootNote;
  int quality;
  int intervals;
};

/*--------------------------------------------------------------------*/
void onset_detected_callback(void* SELF, unsigned long long sample_time)
{
  BTTObject *bttObject = (BTTObject *)SELF;
  bttObject->onsetTimestamps.push_back(sample_time);
}

/*--------------------------------------------------------------------*/
void beat_detected_callback(void* SELF, unsigned long long sample_time)
{
  BTTObject *bttObject = (BTTObject *)SELF;
  bttObject->beatTimestamps.push_back(sample_time);
  // std::cout << "detect beat at " << sample_time << " " << bttObject->beatTimestamps.size() << std::endl;
}

extern "C" {

EMSCRIPTEN_KEEPALIVE BTTObject *createBtt(double sampleRate) {
  BTTObject *bttObject = new BTTObject(sampleRate);
  return bttObject;
}

EMSCRIPTEN_KEEPALIVE void processBt(BTTObject *btt, float *buffer, int buffer_size, BTTProcessOutput *output) {
  btt->onsetTimestamps.clear();
  btt->beatTimestamps.clear();

  {
    btt_process(btt->btt, buffer, buffer_size);
  }
  {
    btt->chromagram->setInputAudioFrameSize(buffer_size);
    // initialize double from floats
    std::vector<double> doubleBuffer(buffer_size);
    for (int i = 0; i < buffer_size; i++) {
      doubleBuffer[i] = buffer[i];
    }
    btt->chromagram->processAudioFrame(doubleBuffer);
  }

  btt->sampleTimestamp += (double)buffer_size;

  output->sampleTimestamp = btt->sampleTimestamp;

  output->bpm = btt_get_tempo_bpm(btt->btt);
  output->numOnsetTimestamps = btt->onsetTimestamps.size();
  output->onsetTimestamps = btt->onsetTimestamps.data();
  output->numBeatTimestamps = btt->beatTimestamps.size();
  output->beatTimestamps = btt->beatTimestamps.data();

  // if (btt->chromagram->isReady()) {
    std::vector<double> chroma = btt->chromagram->getChromagram();
    ChordDetector chordDetector;
    chordDetector.detectChord(chroma);

    output->rootNote = chordDetector.rootNote;
    output->quality = chordDetector.quality;
    output->intervals = chordDetector.intervals;
  // } else {
  //   output->rootNote = -1;
  //   output->quality = -1;
  //   output->intervals = -1;
  // }
}

EMSCRIPTEN_KEEPALIVE void *doMalloc(size_t size) {
  return malloc(size);
}
EMSCRIPTEN_KEEPALIVE void doFree(void *ptr) {
  free(ptr);
}

}

/*--------------------------------------------------------------------*/
/* int main(void)
{
  // instantiate a new object
  BTT* btt = btt_new_default();

  // specify which functions should recieve notificaions
  btt_set_onset_tracking_callback  (btt, onset_detected_callback, nullptr);
  btt_set_beat_tracking_callback   (btt, beat_detected_callback , nullptr);

  int buffer_size = 64;
  dft_sample_t buffer[buffer_size];
  
  for(;;)
  {
    // Fill a buffer with your audio samples here then pass it to btt
    btt_process(btt, buffer, buffer_size);
  }
} */