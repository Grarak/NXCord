#pragma once

#include <zlib.h>

#include <string>

class ZlibWrapper {
 private:
  z_stream _infstream{};

 public:
  ZlibWrapper();

  ~ZlibWrapper();

  std::string decompress(const char *buf, size_t length);
};
