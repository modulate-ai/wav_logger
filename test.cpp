#include "wav_logger.hpp"

#include <cmath>
#include <chrono>
#include <thread>

void test_sync() {
  const int sample_rate = 48000;
  const int buffer_size = 1 * sample_rate;
  WavLogger wav_logger(buffer_size, sample_rate, "./logs", "log");

  // float frequency = 261.626; // middle C
  float frequency = 1.73*261.626;
  size_t num_samples = 480;

  float* audio = new float[num_samples];
  int counter = 0;
  // Test normal operation
  for(size_t k = 0; k < 3; k++) {
    for(size_t j = 0; j < (buffer_size/num_samples)/2; j++) {
      for(size_t i = 0; i < num_samples; i++) {
        audio[i] = sin((2 * M_PI * counter * frequency) / sample_rate);
        counter++;
      }
      bool success = wav_logger.add_audio_nonblocking(audio, num_samples);
      if(!success)
        throw std::runtime_error("Wav logger was blocked while trying to write in sync test");
    }
    wav_logger.write_outstanding_samples_to_file();
  }

  // Test that we can run even when writing is blocked
  for(size_t k = 0; k < 2; k++) {
    for(size_t j = 0; j < (buffer_size/num_samples)/2; j++) {
      for(size_t i = 0; i < num_samples; i++) {
        audio[i] = sin((2 * M_PI * counter * frequency) / sample_rate);
        counter++;
      }
      bool success = wav_logger.add_audio_nonblocking(audio, num_samples);
      if(!success)
        throw std::runtime_error("Wav logger was blocked while trying to write in sync test");
    }
  }
  for(size_t k = 0; k < 2; k++) {
    for(size_t j = 0; j < (buffer_size/num_samples)/2; j++) {
      for(size_t i = 0; i < num_samples; i++) {
        audio[i] = sin((2 * M_PI * counter * frequency) / sample_rate);
        counter++;
      }
      bool success = wav_logger.add_audio_nonblocking(audio, num_samples);
      if(success)
        throw std::runtime_error("Wav logger was blocked while trying to write in sync test");
    }
  }

  // Write out remaining second of audio
  wav_logger.write_outstanding_samples_to_file();
  // And check that multiple calls don't do anything bad
  wav_logger.write_outstanding_samples_to_file();
  wav_logger.write_outstanding_samples_to_file();
  wav_logger.write_outstanding_samples_to_file();

  // Resume normal operation - we should hear a glitch after 2.5s
  for(size_t k = 0; k < 3; k++) {
    for(size_t j = 0; j < (buffer_size/num_samples)/2; j++) {
      for(size_t i = 0; i < num_samples; i++) {
        audio[i] = sin((2 * M_PI * counter * frequency) / sample_rate);
        counter++;
      }
      bool success = wav_logger.add_audio_nonblocking(audio, num_samples);
      if(!success)
        throw std::runtime_error("Wav logger was blocked while trying to write in sync test");
    }
    wav_logger.write_outstanding_samples_to_file();
  }


  // Create multiple logs
  for(size_t k = 0; k < 20; k++) {
    for(size_t j = 0; j < (buffer_size/num_samples)/2; j++) {
      for(size_t i = 0; i < num_samples; i++) {
        audio[i] = sin((2 * M_PI * counter * frequency) / sample_rate);
        counter++;
      }
      bool success = wav_logger.add_audio_nonblocking(audio, num_samples);
      if(!success)
        throw std::runtime_error("Wav logger was blocked while trying to write in sync test");
    }
    wav_logger.write_outstanding_samples_to_file();
  }

  delete[] audio;
}

void logger_write_task(WavLogger* wav_logger_ptr, int sample_rate, int num_seconds) {
  WavLogger& wav_logger = *wav_logger_ptr;
  float frequency = 1.73*261.626;
  size_t num_samples = 480;

  float sec_per_buffer = (float)num_samples / (float)sample_rate;
  int num_iterations = (int)floor((float)num_seconds / sec_per_buffer);

  float* audio = new float[num_samples];
  int counter = 0;
  for(size_t iter = 0; iter < num_iterations; iter++) {
    for(size_t i = 0; i < num_samples; i++) {
      audio[i] = sin((2 * M_PI * counter * frequency) / sample_rate);
      counter++;
    }
    bool success = wav_logger.add_audio_nonblocking(audio, num_samples);
    if(!success)
      throw std::runtime_error("Wav logger was blocked while trying to write in async test");

    std::this_thread::sleep_for(std::chrono::milliseconds((int)floor(sec_per_buffer * 1000)));
  }

  delete[] audio;
}


void read_task(WavLogger* wav_logger_ptr, int buffer_size, int sample_rate,
               float buffer_fraction, bool* should_stop) {
  WavLogger& wav_logger = *wav_logger_ptr;
  float delay_seconds = ((float)buffer_size / (float)sample_rate) * buffer_fraction;

  while(!*should_stop) {
    wav_logger.write_outstanding_samples_to_file();
    std::this_thread::sleep_for(std::chrono::milliseconds((int)floor(delay_seconds * 1000)));
  }

  wav_logger.write_outstanding_samples_to_file();
}


void test_async() {
  const int sample_rate = 48000;
  const int buffer_size = 1 * sample_rate;
  const int num_seconds = 10;
  bool should_stop = false;
  WavLogger wav_logger(buffer_size, sample_rate, "./async_logs", "log");
  auto atomic_thread = std::thread(logger_write_task, &wav_logger, sample_rate, num_seconds);
  auto logger_thread = std::thread(read_task, &wav_logger, buffer_size, sample_rate, 0.25, &should_stop);
  atomic_thread.join();
  should_stop = true;
  logger_thread.join();
}

void threaded_logger_write_task(ThreadedWavLogger* wav_logger_ptr, int starting_sample_rate, int num_iterations) {
  ThreadedWavLogger& wav_logger = *wav_logger_ptr;
  float frequency = 1.73*261.626;
  size_t num_samples = 480;
  int sample_rate = starting_sample_rate;

  float* audio = new float[num_samples];
  int counter = 0;
  for(size_t iter = 0; iter < num_iterations; iter++) {
    if(iter % 100 == 0) {
      if(sample_rate == 48000)
        sample_rate = 44100;
      else
        sample_rate = 48000;
    }

    for(size_t i = 0; i < num_samples; i++) {
      audio[i] = sin((2 * M_PI * counter * frequency) / sample_rate);
      counter++;
    }
    wav_logger.set_sample_rate_nonblocking(sample_rate);
    bool success = wav_logger.add_audio_nonblocking(audio, num_samples);
    if(!success)
      throw std::runtime_error("Wav logger was blocked while trying to write in async test");

    float sec_per_buffer = (float)num_samples / (float)sample_rate;
    std::this_thread::sleep_for(std::chrono::milliseconds((int)floor(sec_per_buffer * 1000)));
  }

  delete[] audio;
}


void test_threaded_logger() {
  const int sample_rate = 48000;
  const int buffer_size = 1 * sample_rate;
  const int num_iterations = 1000;
  ThreadedWavLogger wav_logger2(buffer_size, sample_rate, "./threaded_async_logs", "log");
  wav_logger2.start_logging_thread();
  ThreadedWavLogger wav_logger = std::move(wav_logger2);
  auto atomic_thread = std::thread(threaded_logger_write_task, &wav_logger, sample_rate, num_iterations);
  atomic_thread.join();
  wav_logger.stop_logging_thread();
}


int main() {
  // test_sync();
  // test_async();
  test_threaded_logger();
  return 0;
}
