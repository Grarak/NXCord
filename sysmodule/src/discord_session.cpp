#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include "discord_session.h"
#include "mbedtls_wrapper.h"

#define NEW_LINE "\r\n"

SleepyDiscord::CustomInit SleepyDiscord::Session::init =
    []() -> SleepyDiscord::GenericSession* { return new DiscordSession; };

void DiscordSession::setUrl(const std::string& url) { _url = url; }

void DiscordSession::setBody(const std::string* body) { _body = body; }

void DiscordSession::setHeader(
    const std::vector<SleepyDiscord::HeaderPair>& header) {
  _headers = &header;
}

void DiscordSession::setMultipart(
    const std::initializer_list<SleepyDiscord::Part>& parts) {}

SleepyDiscord::Response DiscordSession::Post() {
  return request(SleepyDiscord::Post);
}

SleepyDiscord::Response DiscordSession::Patch() {
  return {SleepyDiscord::BAD_REQUEST, "", {}};
}

SleepyDiscord::Response DiscordSession::Delete() {
  return {SleepyDiscord::BAD_REQUEST, "", {}};
}

SleepyDiscord::Response DiscordSession::Get() {
  return request(SleepyDiscord::Get);
}

SleepyDiscord::Response DiscordSession::Put() {
  return {SleepyDiscord::BAD_REQUEST, "", {}};
}

SleepyDiscord::Response DiscordSession::request(
    const SleepyDiscord::RequestMethod method) {
  SleepyDiscord::Response response;

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
    return {SleepyDiscord::GENERAL_ERROR,
            "{\"code\":0,\"message\":\"Invalid url\"\"}",
            {}};
  }

  MBedTLSWrapper mbedtls_wrapper(hostname);
  if (!mbedtls_wrapper.usable()) {
    return {
        SleepyDiscord::GENERAL_ERROR,
        "{\"code\":0,\"message\":\"" + mbedtls_wrapper.get_error() + "\"\"}",
        {}};
  }

  struct addrinfo hints {};
  int r, fd;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  struct addrinfo* res;
  r = getaddrinfo(hostname.c_str(), "443", &hints, &res);
  if (r != 0) {
    return {SleepyDiscord::GENERAL_ERROR,
            "{\"code\":0,\"message\":\"getaddrinfo: " +
                std::string(gai_strerror(r)) + "\"}",
            {}};
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
    return {SleepyDiscord::GENERAL_ERROR,
            "{\"code\":431,\"message\":\"Could not connect to the host\"}",
            {}};
  }

  mbedtls_wrapper.register_send_receive(
      &fd,
      [](void* ctx, const unsigned char* buf, size_t len) {
        int* fd = static_cast<int*>(ctx);
        int size = send(*fd, buf, len, 0);
        if (size < 0) {
          printf("Send error %d", size);
          return -1;
        }
        return size;
      },
      [](void* ctx, unsigned char* buf, size_t len) {
        int* fd = static_cast<int*>(ctx);
        int size = recv(*fd, buf, len, 0);
        if (size < 0) {
          printf("Receive error %d", size);
          return -1;
        }
        return size;
      });

  if (!mbedtls_wrapper.start_ssl()) {
    close(fd);
    return {
        SleepyDiscord::GENERAL_ERROR,
        "{\"code\":431,\"message\":\"" + mbedtls_wrapper.get_error() + "\"}",
        {}};
  }

  const char* method_names[] = {"POST", "PATCH", "DELETE", "GET", "PUT"};
  std::string metadata = method_names[method];
  metadata.append(" ").append(path).append(" HTTP/1.1").append(NEW_LINE);
  metadata.append("Host: ").append(hostname).append(NEW_LINE);
  metadata.append("Accept: */*").append(NEW_LINE);
  metadata.append("Connection: close").append(NEW_LINE);

  if (_body) {
    metadata.append("Content-Length: ").append(std::to_string(_body->size()));
  }

  if (_headers) {
    for (const SleepyDiscord::HeaderPair& pair : *_headers) {
      metadata.append(pair.name)
          .append(": ")
          .append(pair.value)
          .append(NEW_LINE);
    }
  }
  metadata.append(NEW_LINE);

  if (!mbedtls_wrapper.write(metadata.c_str(), metadata.size())) {
    close(fd);
    return {
        SleepyDiscord::GENERAL_ERROR,
        "{\"code\":431,\"message\":\"" + mbedtls_wrapper.get_error() + "\"}",
        {}};
  }
  if (_body) {
    if (!mbedtls_wrapper.write(_body->c_str(), _body->size())) {
      close(fd);
      return {
          SleepyDiscord::GENERAL_ERROR,
          "{\"code\":431,\"message\":\"" + mbedtls_wrapper.get_error() + "\"}",
          {}};
    }
  }

  size_t buf_size = 1024;
  char buf[buf_size];
  bool collect_headers = true;
  std::string header_buf;
  size_t ret;
  while ((ret = mbedtls_wrapper.read(buf, buf_size)) > 0) {
    buf[ret] = 0;

    if (collect_headers) {
      size_t header_start = 0;
      size_t header_end = 0;

      for (; header_end < ret; ++header_end) {
        if (buf[header_end] == '\r') {
          if (header_buf.empty() && header_start == header_end) {
            collect_headers = false;
            if (header_end + 2 < ret) {
              response.text.insert(response.text.end(), buf + header_end + 2,
                                   buf + ret);
            }
            break;
          }

          std::string new_header = header_buf;
          new_header.insert(new_header.end(), buf + header_start,
                            buf + header_end);
          if (response.header.empty() && new_header.substr(0, 4) == "HTTP") {
            response.statusCode = stoi(new_header.substr(9, 3));
          } else {
            size_t key_size = new_header.find(": ");
            if (key_size > 0) {
              std::string key = new_header.substr(0, key_size);
              std::string value =
                  new_header.substr(key_size + 2, new_header.size());
              response.header[std::move(key)] = std::move(value);
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
      response.text.insert(response.text.end(), buf,
                           buf + ret);  // TODO use content length if possible
    }

    if (ret < buf_size) {
      break;
    }
  }

  close(fd);

  _body = nullptr;
  _headers = nullptr;
  return response;
}
