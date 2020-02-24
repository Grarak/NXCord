#pragma once

#include <zlib.h>

#include <fstream>

class ZlibWrapper {
 private:
  std::istream *_stream = nullptr;
  std::streamsize _stream_read_size = 0;
  char _in_buf[1024]{};
  z_stream _infstream{};

 public:
  ZlibWrapper();

  ~ZlibWrapper();

  void set_stream(std::istream *stream);

  int64_t read(char *buf, size_t size);
};
