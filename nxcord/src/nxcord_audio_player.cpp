#include <malloc.h>

#include <common/logger.hpp>
#include <nxcord/nxcord_audio_player.hpp>
#include <set>

void player_thread(NXCordAudioPlayer *audio_player) {
  size_t frame_size = SleepyDiscord::AudioTransmissionDetails::proposedLength();

  auto *buf = reinterpret_cast<SleepyDiscord::AudioSample *>(
      audio_player->_audout_buf.buffer);

  time_t current_time = Utils::current_time_millis();

  {
    std::set<uint32_t> empty_ssrc;
    std::vector<AudioPacket> audio_sources;

    {
      std::scoped_lock lock(audio_player->_queue_mutex);

      for (auto &audio_pair : audio_player->_queue) {
        if (audio_pair.second.empty()) {
          empty_ssrc.insert(audio_pair.first);
          continue;
        }

        AudioPacket *packet = &audio_pair.second.front();
        while (current_time - packet->time >
               100) {  // Make sure packet is not older than 100ms
          audio_pair.second.pop();
          if (audio_pair.second.empty()) {
            packet = nullptr;
            break;
          }
          packet = &audio_pair.second.front();
        }

        if (!packet) {
          empty_ssrc.insert(audio_pair.first);
          continue;
        }

        audio_sources.push_back(std::move(*packet));
        audio_pair.second.pop();
        if (audio_pair.second.empty()) {
          empty_ssrc.insert(audio_pair.first);
        }
      }

      for (uint32_t ssrc : empty_ssrc) {
        audio_player->_queue.erase(ssrc);
      }
    }

    for (size_t i = 0; i < frame_size; ++i) {
      int sample = 0;
      for (const auto &packet : audio_sources) {
        if (i < packet.data.size()) {
          SleepyDiscord::AudioSample packet_sample = packet.data[i];
          {
            std::shared_lock lock(audio_player->_ssrc_multipliers_mutex);
            auto it = audio_player->_ssrc_multipliers.find(packet.ssrc);
            if (it != audio_player->_ssrc_multipliers.end()) {
              packet_sample *= it->second;
            }
          }
          sample += packet_sample;
        }
      }
      if (sample > Utils::audio_sample_max) {
        buf[i] = Utils::audio_sample_max;
      } else if (sample < Utils::audio_sample_min) {
        buf[i] = Utils::audio_sample_min;
      } else {
        buf[i] = static_cast<SleepyDiscord::AudioSample>(sample);
      }
    }
  }

  AudioOutBuffer *audout_released_buf = nullptr;
  audoutPlayBuffer(&audio_player->_audout_buf, &audout_released_buf);

  time_t time_passed = Utils::current_time_millis() - current_time;
  if (time_passed < 20) {
    svcSleepThread((20L - time_passed) * 1e+6L);
  }
}

NXCordAudioPlayer::NXCordAudioPlayer()
    : _player_thread(this, player_thread, 0x4000) {
  Result rc = audoutStartAudioOut();
  Logger::write("NXCordAudioPlayer: audoutStartAudioOut() returned 0x%x\n", rc);

  size_t buffer_size = (Utils::opus_framesize_bytes + 0xfff) & ~0xfff;
  void *out_buf_data = memalign(0x1000, buffer_size);
  std::memset(out_buf_data, 0, buffer_size);

  _audout_buf.next = nullptr;
  _audout_buf.buffer = out_buf_data;
  _audout_buf.buffer_size = buffer_size;
  _audout_buf.data_size = Utils::opus_framesize_bytes;
  _audout_buf.data_offset = 0;

  rc = audoutAppendAudioOutBuffer(&_audout_buf);
  Logger::write(
      "NXCordAudioPlayer: audoutAppendAudioOutBuffer() returned 0x%x\n", rc);

  _player_thread.start();
}

NXCordAudioPlayer::~NXCordAudioPlayer() {
  Logger::write("NXCordAudioPlayer: Releasing audio receiver\n");
  _player_thread.stop();

  Result rc = audoutStopAudioOut();
  Logger::write("NXCordAudioPlayer: audoutStopAudioOut() returned 0x%x\n", rc);
}

void NXCordAudioPlayer::queue(
    std::vector<SleepyDiscord::AudioSample> &audio,
    const SleepyDiscord::AudioTransmissionDetails &details) {
  std::scoped_lock lock(_queue_mutex);
  std::queue<AudioPacket> &queue = _queue[details.ssrc()];
  while (queue.size() > 5) {
    queue.pop();
  }
  queue.push(AudioPacket{details.ssrc(), audio});
}

void NXCordAudioPlayer::setSSRCMultiplier(uint32_t ssrc, float multiplier) {
  std::unique_lock lock(_ssrc_multipliers_mutex);
  _ssrc_multipliers[ssrc] = multiplier;
}

float NXCordAudioPlayer::getSSRCMultiplier(uint32_t ssrc) const {
  std::shared_lock lock(_ssrc_multipliers_mutex);
  auto it = _ssrc_multipliers.find(ssrc);
  if (it != _ssrc_multipliers.end()) {
    return it->second;
  }
  return 1;
}
