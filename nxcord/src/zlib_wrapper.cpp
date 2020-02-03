#include <common/logger.hpp>
#include <nxcord/zlib_wrapper.hpp>

ZlibWrapper::ZlibWrapper() {
  _infstream.zalloc = Z_NULL;
  _infstream.zfree = Z_NULL;

  inflateInit2(&_infstream, MAX_WBITS + 32);
}

ZlibWrapper::~ZlibWrapper() { inflateEnd(&_infstream); }

std::string ZlibWrapper::decompress(const char *compressed, size_t length) {
  std::string decompressed;
  char buf[1024];

  _infstream.avail_in = length;
  _infstream.next_in = (Bytef *)compressed;

  while (true) {
    _infstream.avail_out = sizeof(buf);
    _infstream.next_out = reinterpret_cast<Bytef *>(buf);
    _infstream.total_out = 0;

    int ret = inflate(&_infstream, Z_SYNC_FLUSH);
    decompressed.insert(decompressed.end(), buf, buf + _infstream.total_out);

    if (ret == Z_STREAM_END || ret == Z_BUF_ERROR) {
      break;
    } else if (ret != Z_OK) {
      Logger::write("Failed to decompress zlib %d\n", ret);
      return decompressed;
    }
  }
  return decompressed;
}
