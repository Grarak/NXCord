#pragma once

#include <sleepy_discord/sleepy_discord.h>
#include <switch.h>

#include <atomic>
#include <common/loop_thread.hpp>
#include <common/utils.hpp>
#include <mutex>
#include <queue>

#include "audio_packet.hpp"
#include "nxcord_client.hpp"

class NXCordAudioPlayer {
 private:
  AudioOutBuffer _audout_buf{};
  std::map<uint32_t, std::queue<AudioPacket>> _queue;
  std::mutex _queue_mutex;

  LoopThread<NXCordAudioPlayer *> _player_thread;

  std::map<uint32_t, float> _ssrc_multipliers;
  mutable std::shared_mutex _ssrc_multipliers_mutex;

  friend void player_thread(NXCordAudioPlayer *audio_player);

 public:
  NXCordAudioPlayer();

  ~NXCordAudioPlayer();

  void queue(std::vector<SleepyDiscord::AudioSample> &audio,
             const SleepyDiscord::AudioTransmissionDetails &details);

  void setSSRCMultiplier(uint32_t ssrc, float multiplier);
  float getSSRCMultiplier(uint32_t ssrc) const;
};
