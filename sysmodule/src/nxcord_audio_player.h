#pragma once

#include <sleepy_discord/sleepy_discord.h>
#include <switch.h>

#include <atomic>
#include <mutex>
#include <queue>

#include "audio_packet.h"
#include "loop_thread.h"
#include "nxcord_client.h"
#include "utils.h"

class NXCordAudioPlayer {
 private:
  AudioOutBuffer _audout_buf;
  std::map<uint32_t, std::queue<AudioPacket>> _queue;
  std::mutex _queue_mutex;

  LoopThread<NXCordAudioPlayer*> _player_thread;

  friend void player_thread(NXCordAudioPlayer* audio_player);

 public:
  NXCordAudioPlayer();
  ~NXCordAudioPlayer();

  void queue(std::vector<SleepyDiscord::AudioSample>& audio,
             const SleepyDiscord::AudioTransmissionDetails& details);
};
