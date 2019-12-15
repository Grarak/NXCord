#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <zlib.h>

#include "discord_session.h"

#define NEW_LINE "\r\n"
constexpr const char* METHOD_NAMES[] = {"POST", "PATCH", "DELETE", "GET",
                                        "PUT"};

SleepyDiscord::CustomInit SleepyDiscord::Session::init =
    []() -> SleepyDiscord::GenericSession* { return new DiscordSession; };

void DiscordSession::setUrl(const std::string& url) { _url = url; }

void DiscordSession::setBody(const std::string* body) { _body = body; }

void DiscordSession::setHeader(
    const std::vector<SleepyDiscord::HeaderPair>& header) {
  _headers = &header;
}

bool DiscordSession::contains_header(const std::string& key) const {
  if (!_headers) {
    return false;
  }
  for (const SleepyDiscord::HeaderPair& pair : *_headers) {
    if (pair.name == key) {
      return true;
    }
  }
  return false;
}

std::unique_ptr<MBedTLSWrapper> DiscordSession::request(
    const SleepyDiscord::RequestMethod method,
    SleepyDiscord::Response* response) {
  printf("Requesting %s %s\n", _url.c_str(), METHOD_NAMES[method]);

  int ret;
  std::string hostname, path;
  std::string protocol = _url.substr(0, _url.find("://"));
  size_t offset = protocol.length() + 3;
  if (offset < _url.length()) {  // makes sure that there is a hostname
    hostname = _url.substr(offset, _url.find("/", offset) - offset);

    offset += hostname.length();
    if (_url[offset] == '/') {
      path = _url.substr(offset);
    }
  } else {
    response->statusCode = SleepyDiscord::GENERAL_ERROR;
    response->text = "{\"code\":0,\"message\":\"Invalid url\"\"}";
    return nullptr;
  }

  auto mbedtls_wrapper = std::make_unique<MBedTLSWrapper>(hostname);
  if (!mbedtls_wrapper->usable()) {
    response->statusCode = SleepyDiscord::GENERAL_ERROR;
    response->text =
        "{\"code\":0,\"message\":\"" + mbedtls_wrapper->get_error() + "\"\"}";
    return mbedtls_wrapper;
  }

  struct addrinfo hints {};
  int r, fd;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  struct addrinfo* res;

  printf("Connecting to %s\n", hostname.c_str());
  r = getaddrinfo(hostname.c_str(), "443", &hints, &res);
  if (r != 0) {
    response->statusCode = SleepyDiscord::GENERAL_ERROR;
    response->text = "{\"code\":0,\"message\":\"getaddrinfo: " +
                     std::string(gai_strerror(r)) + "\"}";
    return mbedtls_wrapper;
  }

  for (struct addrinfo* rp = res; rp; rp = rp->ai_next) {
    fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (fd == -1) {
      continue;
    }
    while ((r = connect(fd, rp->ai_addr, rp->ai_addrlen)) == -1 &&
           errno == EINTR)
      ;
    if (r == 0) {
      break;
    }
    close(fd);
    fd = -1;
  }
  freeaddrinfo(res);
  if (fd == -1) {  // error check
    response->statusCode = SleepyDiscord::GENERAL_ERROR;
    response->text =
        "{\"code\":431,\"message\":\"Could not connect to the host\"}";
    return mbedtls_wrapper;
  }

  printf("Connection established %d\n", fd);
  mbedtls_wrapper->set_fd(fd);

  printf("Start handshake\n");
  if (!mbedtls_wrapper->start_ssl()) {
    response->statusCode = SleepyDiscord::GENERAL_ERROR;
    response->text =
        "{\"code\":431,\"message\":\"" + mbedtls_wrapper->get_error() + "\"}";
    return mbedtls_wrapper;
  }

  std::string metadata = METHOD_NAMES[method];
  metadata.append(" ").append(path).append(" HTTP/1.0").append(NEW_LINE);
  metadata.append("Host: ").append(hostname).append(":443").append(NEW_LINE);

  if (!contains_header("Accept")) {
    metadata.append("Accept: */*").append(NEW_LINE);
  }
  if (!contains_header("Connection")) {
    metadata.append("Connection: close").append(NEW_LINE);
  }
  if (!contains_header("Accept-Encoding")) {
    metadata.append("Accept-Encoding: gzip").append(NEW_LINE);
  }

  if (_headers) {
    for (const SleepyDiscord::HeaderPair& pair : *_headers) {
      metadata.append(pair.name)
          .append(": ")
          .append(pair.value)
          .append(NEW_LINE);
    }
  }
  if (_body && !contains_header("Content-Length")) {
    metadata.append("Content-Length: ").append(std::to_string(_body->size()));
  }
  metadata.append(NEW_LINE);

  printf("Sending payload\n");

  ret = mbedtls_wrapper->write(
      reinterpret_cast<const unsigned char*>(metadata.c_str()),
      metadata.size());
  if (ret < 0) {
    response->statusCode = SleepyDiscord::GENERAL_ERROR;
    response->text = "{\"code\":431,\"message\":\"Write error " +
                     std::to_string(ret) + "\"}";
    return mbedtls_wrapper;
  }
  if (_body) {
    ret = mbedtls_wrapper->write(
        reinterpret_cast<const unsigned char*>(_body->c_str()), _body->size());
    if (ret < 0) {
      response->statusCode = SleepyDiscord::GENERAL_ERROR;
      response->text = "{\"code\":431,\"message\":\"Write error " +
                       std::to_string(ret) + "\"}";
      return mbedtls_wrapper;
    }
  }

  printf("Reading response\n");
  int buf_size = 4096;
  unsigned char buf[buf_size];
  bool collect_headers = true;
  std::string header_buf;
  int read_len;
  while ((read_len = mbedtls_wrapper->read(buf, buf_size)) > 0) {
    buf[read_len] = 0;

    if (collect_headers) {
      int header_start = 0;
      int header_end = 0;

      for (; header_end < read_len; ++header_end) {
        if (buf[header_end] == '\r') {
          if (header_buf.empty() && header_start == header_end) {
            collect_headers = false;
            if (header_end + 2 < read_len) {
              response->text.insert(response->text.end(), buf + header_end + 2,
                                    buf + read_len);
            }
            break;
          }

          std::string new_header = header_buf;
          new_header.insert(new_header.end(), buf + header_start,
                            buf + header_end);
          if (response->header.empty() && new_header.substr(0, 4) == "HTTP") {
            response->statusCode = stoi(new_header.substr(9, 3));
          } else {
            size_t key_size = new_header.find(": ");
            if (key_size > 0) {
              std::string key = new_header.substr(0, key_size);
              std::string value =
                  new_header.substr(key_size + 2, new_header.size());
              response->header[std::move(key)] = std::move(value);
            }
          }
          header_buf.clear();
          header_start = ++header_end;  // Skip \n
          ++header_start;               // Skip \n
          continue;
        }

        if (buf[header_end] == '\n') {
          ++header_start;  // Skip \n
        }
      }

      if (collect_headers && header_start < header_end) {
        std::string test;
        test.insert(test.end(), buf + header_start, buf + header_end);
        header_buf.insert(header_buf.end(), buf + header_start,
                          buf + header_end);
      }
    } else {
      response->text.insert(
          response->text.end(), buf,
          buf + read_len);  // TODO use content length if possible
    }

    if (read_len < buf_size) {
      break;
    }
  }

  auto it = response->header.find("Content-Encoding");
  if (it != response->header.end() && it->second == "gzip") {
    std::string decompressed;
    z_stream infstream;
    infstream.zalloc = Z_NULL;
    infstream.zfree = Z_NULL;
    infstream.avail_in = response->text.size();  // size of input + 1 for
    infstream.next_in = (Bytef*)response->text.c_str();

    inflateInit2(&infstream, (16 + MAX_WBITS));
    while (true) {
      infstream.avail_out = buf_size;
      infstream.next_out = (Bytef*)buf;
      infstream.total_out = 0;

      ret = inflate(&infstream, Z_SYNC_FLUSH);
      decompressed.insert(decompressed.end(), buf, buf + infstream.total_out);

      if (ret == Z_STREAM_END) {
        break;
      } else if (ret != Z_OK) {
        response->text =
            "{\"code\":431,\"message\":\"Couldn't decompress body\"}";
        return mbedtls_wrapper;
      }
    }
    inflateEnd(&infstream);
    response->text = std::move(decompressed);
  }

  _body = nullptr;
  _headers = nullptr;
  return mbedtls_wrapper;
}

SleepyDiscord::Response DiscordSession::request(
    SleepyDiscord::RequestMethod method) {
  SleepyDiscord::Response response;
  request(method, &response);
  return response;
}
