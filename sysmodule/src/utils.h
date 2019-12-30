#pragma once

#include <sleepy_discord/voice_connection.h>

#include <chrono>

namespace Utils {

constexpr size_t opus_framesize_bytes =
    SleepyDiscord::AudioTransmissionDetails::proposedLength() *
    sizeof(SleepyDiscord::AudioSample);

constexpr SleepyDiscord::AudioSample audio_sample_max =
    std::numeric_limits<SleepyDiscord::AudioSample>::max();

constexpr SleepyDiscord::AudioSample audio_sample_min =
    std::numeric_limits<SleepyDiscord::AudioSample>::min();

const time_t current_time_millis();

}  // namespace Utils
