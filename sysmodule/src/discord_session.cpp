#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

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

void DiscordSession::setMultipart(
    const std::initializer_list<SleepyDiscord::Part>& parts) {}

std::shared_ptr<MBedTLSWrapper> DiscordSession::request(
    const SleepyDiscord::RequestMethod method,
    SleepyDiscord::Response* response) {
  printf("Requesting %s %s\n", _url.c_str(), METHOD_NAMES[method]);

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

  auto mbedtls_wrapper = std::make_shared<MBedTLSWrapper>(hostname);
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

  printf("Sending payload\n");
  if (!mbedtls_wrapper->write(metadata.c_str(), metadata.size())) {
    response->statusCode = SleepyDiscord::GENERAL_ERROR;
    response->text =
        "{\"code\":431,\"message\":\"" + mbedtls_wrapper->get_error() + "\"}";
    return mbedtls_wrapper;
  }
  if (_body) {
    if (!mbedtls_wrapper->write(_body->c_str(), _body->size())) {
      response->statusCode = SleepyDiscord::GENERAL_ERROR;
      response->text =
          "{\"code\":431,\"message\":\"" + mbedtls_wrapper->get_error() + "\"}";
      return mbedtls_wrapper;
    }
  }

  printf("Reading response\n");
  size_t buf_size = 1024;
  char buf[buf_size];
  bool collect_headers = true;
  std::string header_buf;
  size_t ret;
  while ((ret = mbedtls_wrapper->read(buf, buf_size)) > 0) {
    buf[ret] = 0;

    if (collect_headers) {
      size_t header_start = 0;
      size_t header_end = 0;

      for (; header_end < ret; ++header_end) {
        if (buf[header_end] == '\r') {
          if (header_buf.empty() && header_start == header_end) {
            collect_headers = false;
            if (header_end + 2 < ret) {
              response->text.insert(response->text.end(), buf + header_end + 2,
                                    buf + ret);
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
      response->text.insert(response->text.end(), buf,
                            buf + ret);  // TODO use content length if possible
    }

    if (ret < buf_size) {
      break;
    }
  }

  printf("Closing connection to %s\n", hostname.c_str());

  _body = nullptr;
  _headers = nullptr;
  return mbedtls_wrapper;
}

SleepyDiscord::Response DiscordSession::Post() {
  SleepyDiscord::Response response;
  request(SleepyDiscord::Post, &response);
  return response;
}

SleepyDiscord::Response DiscordSession::Patch() {
  return {SleepyDiscord::BAD_REQUEST, "", {}};
}

SleepyDiscord::Response DiscordSession::Delete() {
  return {SleepyDiscord::BAD_REQUEST, "", {}};
}

SleepyDiscord::Response DiscordSession::Get() {
  SleepyDiscord::Response response;
  request(SleepyDiscord::Get, &response);
  return response;
}

SleepyDiscord::Response DiscordSession::Put() {
  return {SleepyDiscord::BAD_REQUEST, "", {}};
}
