#include <malloc.h>

#include "logger.h"
#include "nxcord_audio_capture.h"
#include "utils.h"

void capture_thread(NXCordAudioCapture* audio_capture) {
  uint32_t released_in_count;
  AudioInBuffer* audin_released_buf = nullptr;

  time_t current_time = Utils::current_time_millis();

  audinWaitCaptureFinish(&audin_released_buf, &released_in_count, U64_MAX);
  if (released_in_count > 0) {
    if (audin_released_buf) {
      std::vector<SleepyDiscord::AudioSample> capture;
      capture.resize(audin_released_buf->data_size /
                     sizeof(SleepyDiscord::AudioSample));
      std::memcpy(capture.data(), audin_released_buf->buffer,
                  audin_released_buf->data_size);

      AudioPacket packet(0, capture);
      audio_capture->_queue_mutex.lock();
      while (audio_capture->_queue.size() > 10) {
        audio_capture->_queue.pop();
      }
      audio_capture->_queue.push(std::move(packet));
      audio_capture->_queue_mutex.unlock();
    }

    std::memset(audin_released_buf->buffer, 0, audin_released_buf->data_size);
    audinAppendAudioInBuffer(audin_released_buf);
  }

  time_t time_passed = Utils::current_time_millis() - current_time;
  if (time_passed < 20) {
    svcSleepThread((20L - time_passed) * 1e+6L);
  }
}

NXCordAudioCapture::NXCordAudioCapture()
    : _capture_thread(this, capture_thread, 0x4000) {
  Result rc = audinStartAudioIn();
  Logger::write("audinStartAudioIn() returned 0x%x\n", rc);

  u32 buffer_size = (Utils::opus_framesize_bytes + 0xfff) & ~0xfff;
  void* in_buf_data = memalign(0x1000, buffer_size);
  std::memset(in_buf_data, 0, buffer_size);

  _audin_buf.next = NULL;
  _audin_buf.buffer = in_buf_data;
  _audin_buf.buffer_size = buffer_size;
  _audin_buf.data_size = Utils::opus_framesize_bytes;
  _audin_buf.data_offset = 0;

  rc = audinAppendAudioInBuffer(&_audin_buf);
  Logger::write("audinAppendAudioInBuffer() returned 0x%x\n", rc);

  _capture_thread.start();
}

NXCordAudioCapture::~NXCordAudioCapture() {
  Logger::write("Releasing audio sender\n");
  _capture_thread.stop();
  uint32_t released_in_count;
  AudioInBuffer* audin_released_buf = nullptr;
  audinWaitCaptureFinish(&audin_released_buf, &released_in_count, U64_MAX);

  Result rc = audinStopAudioIn();
  Logger::write("audinStopAudioIn() returned 0x%x\n", rc);
}

std::vector<SleepyDiscord::AudioSample> NXCordAudioCapture::poll() {
  time_t current_time = Utils::current_time_millis();
  std::lock_guard<std::mutex> lock(_queue_mutex);
  std::vector<SleepyDiscord::AudioSample> capture;

  if (!_queue.empty()) {
    AudioPacket* packet = &_queue.front();
    while (current_time - packet->time > 100) {
      _queue.pop();
      if (_queue.empty()) {
        return capture;
      }
      packet = &_queue.front();
    }
    capture = std::move(packet->data);
    _queue.pop();
  }
  return capture;
}
