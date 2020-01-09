#pragma once

#include <chrono>
#include <string>

namespace Utils {

constexpr size_t opus_framesize_bytes =
    960 * 2 * sizeof(int16_t);  // frame size * channels * sample size

constexpr int16_t audio_sample_max = std::numeric_limits<int16_t>::max();

constexpr int16_t audio_sample_min = std::numeric_limits<int16_t>::min();

const time_t current_time_millis();

const bool create_directories(const char* file_path);

const void copy_file(const std::string& srcPath, const std::string& destPath);

const bool file_exists(const std::string& path);

}  // namespace Utils
