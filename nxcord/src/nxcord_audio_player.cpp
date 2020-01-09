#include <malloc.h>

#include <common/logger.hpp>
#include <limits>
#include <nxcord/nxcord_audio_player.hpp>
#include <set>

void player_thread(NXCordAudioPlayer* audio_player) {
  size_t frame_size = SleepyDiscord::AudioTransmissionDetails::proposedLength();

  auto* buf = reinterpret_cast<SleepyDiscord::AudioSample*>(
      audio_player->_audout_buf.buffer);

  time_t current_time = Utils::current_time_millis();

  {
    std::set<uint32_t> empty_ssrc;
    std::vector<AudioPacket> audio_sources;

    audio_player->_queue_mutex.lock();

    for (auto it = audio_player->_queue.begin();
         it != audio_player->_queue.end(); ++it) {
      if (it->second.empty()) {
        empty_ssrc.insert(it->first);
        continue;
      }

      AudioPacket* packet = &it->second.front();
      while (current_time - packet->time >
             100) {  // Make sure packet is not older than 100ms
        it->second.pop();
        if (it->second.empty()) {
          packet = nullptr;
          break;
        }
        packet = &it->second.front();
      }

      if (!packet) {
        empty_ssrc.insert(it->first);
        continue;
      }

      audio_sources.push_back(std::move(*packet));
      it->second.pop();
      if (it->second.empty()) {
        empty_ssrc.insert(it->first);
      }
    }

    for (uint32_t ssrc : empty_ssrc) {
      audio_player->_queue.erase(ssrc);
    }
    audio_player->_queue_mutex.unlock();

    for (size_t i = 0; i < frame_size; ++i) {
      int sample = 0;
      for (const auto& packet : audio_sources) {
        if (i < packet.data.size()) {
          sample += packet.data[i];
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

  /*std::vector<SleepyDiscord::AudioSample> audio_vector(buf, buf +
  frame_size); audio_player->client->audio_mutex.lock();
  audio_player->client->audio_out.push(std::move(audio_vector));
  audio_player->client->audio_mutex.unlock();*/

  AudioOutBuffer* audout_released_buf = nullptr;
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
  void* out_buf_data = memalign(0x1000, buffer_size);
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
    std::vector<SleepyDiscord::AudioSample>& audio,
    const SleepyDiscord::AudioTransmissionDetails& details) {
  std::scoped_lock lock(_queue_mutex);
  std::queue<AudioPacket>& queue = _queue[details.ssrc()];
  while (queue.size() > 10) {
    queue.pop();
  }
  queue.push(AudioPacket{details.ssrc(), audio});
}
