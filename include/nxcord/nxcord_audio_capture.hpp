#pragma once

#include <sleepy_discord/sleepy_discord.h>
#include <switch.h>

#include <common/loop_thread.hpp>
#include <mutex>
#include <queue>

#include "audio_packet.hpp"

class NXCordAudioCapture {
 private:
  AudioInBuffer _audin_buf{};

  std::mutex _queue_mutex;
  std::queue<AudioPacket> _queue;

  LoopThread<NXCordAudioCapture *> _capture_thread;

  friend void capture_thread(NXCordAudioCapture *audio_capture);

 public:
  NXCordAudioCapture();

  ~NXCordAudioCapture();

  std::vector<SleepyDiscord::AudioSample> poll();
};
