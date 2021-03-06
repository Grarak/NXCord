#pragma once

#include <sleepy_discord/sleepy_discord.h>

#include <common/utils.hpp>

struct AudioPacket {
  uint32_t ssrc;
  time_t time;
  std::vector<SleepyDiscord::AudioSample> data;

  AudioPacket(uint32_t ssrc, std::vector<SleepyDiscord::AudioSample> &data)
      : ssrc(ssrc), time(Utils::current_time_millis()), data(std::move(data)) {}

  AudioPacket(AudioPacket &&other) noexcept
      : ssrc(other.ssrc), time(other.time), data(std::move(other.data)) {}
};
