#pragma once

#include <sleepy_discord/sleepy_discord.h>

#include "zlib_wrapper.hpp"

class WebsocketZlibStream : public SleepyDiscord::JsonInputStream {
 public:
  typedef typename rapidjson::UTF8<>::Ch Ch;

 private:
  ZlibWrapper &_zlib_wrapper;
  char _buf[1024]{};
  int64_t _buf_read_size = 0;
  uint32_t _current_index = 0;
  size_t _total_read = 0;

  bool _cache_enabled = false;
  std::string _cache;

 public:
  explicit WebsocketZlibStream(ZlibWrapper &zlib_wrapper)
      : _zlib_wrapper(zlib_wrapper) {
    _buf_read_size = zlib_wrapper.read(_buf, sizeof(_buf));
  }

  [[nodiscard]] inline Ch Peek() const override {
    return _current_index >= _buf_read_size ? '\0' : _buf[_current_index];
  }

  inline Ch Take() override {
    if (_current_index >= _buf_read_size) {
      return '\0';
    }
    Ch ret = _buf[_current_index];
    ++_current_index;
    if (_current_index >= _buf_read_size) {
      _buf_read_size = _zlib_wrapper.read(_buf, sizeof(_buf));
      _current_index = 0;
    }

    if (_cache_enabled) {
      _cache.push_back(ret);
    }
    return ret;
  }

  [[nodiscard]] inline size_t Tell() const override {
    return _total_read - _buf_read_size + _current_index;
  }

  inline void SetCache(bool enable) override {
    _cache.clear();
    _cache.shrink_to_fit();
    _cache_enabled = enable;
  }

  [[nodiscard]] inline const std::string &GetCache() const override {
    return _cache;
  }
};
