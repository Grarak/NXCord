#include <http_parser.h>
#include <netdb.h>
#include <switch.h>
#include <sys/socket.h>
#include <unistd.h>

#include <common/logger.hpp>
#include <nxcord/discord_session.hpp>
#include <nxcord/zlib_wrapper.hpp>
#include <sstream>

#define NEW_LINE "\r\n"
constexpr const char *METHOD_NAMES[] = {"POST", "PATCH", "DELETE", "GET",
                                        "PUT"};
constexpr const char HTTP_CHUNK_TERMINATION[] = {0x30, '\r', '\n', '\r', '\n'};

SleepyDiscord::CustomInit SleepyDiscord::Session::init =
    []() -> SleepyDiscord::GenericSession * { return new DiscordSession; };

void DiscordSession::setUrl(const std::string &url) { _url = url; }

void DiscordSession::setBody(const std::string *body) { _body = body; }

void DiscordSession::setHeader(
    const std::vector<SleepyDiscord::HeaderPair> &header) {
  _headers = &header;
}

bool DiscordSession::contains_header(const std::string &key) const {
  if (!_headers) {
    return false;
  }
  for (const SleepyDiscord::HeaderPair &pair : *_headers) {
    if (pair.name == key) {
      return true;
    }
  }
  return false;
}

std::unique_ptr<MBedTLSWrapper> DiscordSession::request(
    const SleepyDiscord::RequestMethod method,
    SleepyDiscord::Response *response) {
  Logger::write("Requesting %s %s\n", _url.c_str(), METHOD_NAMES[method]);

  int ret;
  std::string hostname, path;
  std::string protocol = _url.substr(0, _url.find("://"));
  size_t offset = protocol.length() + 3;
  if (offset < _url.length()) {  // makes sure that there is a hostname
    hostname = _url.substr(offset, _url.find('/', offset) - offset);

    offset += hostname.length();
    if (_url[offset] == '/') {
      path = _url.substr(offset);
    }
  } else {
    response->statusCode = SleepyDiscord::GENERAL_ERROR;
    response->text = R"({"code":0,"message":"Invalid url""})";
    return nullptr;
  }

  auto mbedtls_wrapper = std::make_unique<MBedTLSWrapper>(hostname);
  if (!mbedtls_wrapper->usable()) {
    response->statusCode = SleepyDiscord::GENERAL_ERROR;
    response->text =
        R"({"code":0,"message":")" + mbedtls_wrapper->getError() + "\"\"}";
    return mbedtls_wrapper;
  }

  addrinfo hints{};
  int fd = -1;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  addrinfo *res;

  Logger::write("Connecting to %s\n", hostname.c_str());
  ret = getaddrinfo(hostname.c_str(), "443", &hints, &res);
  if (ret != 0) {
    response->statusCode = SleepyDiscord::GENERAL_ERROR;
    response->text = R"({"code":0,"message":"getaddrinfo: )" +
                     std::string(gai_strerror(ret)) + "\"}";
    return mbedtls_wrapper;
  }

  for (addrinfo *rp = res; rp; rp = rp->ai_next) {
    fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (fd == -1) {
      continue;
    }
    while ((ret = connect(fd, rp->ai_addr, rp->ai_addrlen)) == -1 &&
           errno == EINTR)
      ;
    if (ret == 0) {
      break;
    }
    close(fd);
    fd = -1;
  }
  freeaddrinfo(res);
  if (fd == -1) {  // error check
    response->statusCode = SleepyDiscord::GENERAL_ERROR;
    response->text =
        R"({"code":431,"message":"Could not connect to the host"})";
    return mbedtls_wrapper;
  }

  Logger::write("Connection established %d\n", fd);
  mbedtls_wrapper->setFd(fd);

  Logger::write("Start handshake\n");
  if (!mbedtls_wrapper->startSSL()) {
    response->statusCode = SleepyDiscord::GENERAL_ERROR;
    response->text =
        R"({"code":431,"message":")" + mbedtls_wrapper->getError() + "\"}";
    return mbedtls_wrapper;
  }

  std::string metadata = METHOD_NAMES[method];
  metadata.append(" ").append(path).append(" HTTP/1.1").append(NEW_LINE);
  metadata.append("Host: ").append(hostname).append(NEW_LINE);

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
    for (const auto &pair : *_headers) {
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

  if (_body) {
    metadata.append(NEW_LINE);
  }

  Logger::write("Sending payload\n");
  ret = mbedtls_wrapper->write(
      reinterpret_cast<const unsigned char *>(metadata.c_str()),
      metadata.size());
  if (ret < 0) {
    response->statusCode = SleepyDiscord::GENERAL_ERROR;
    response->text =
        R"({"code":431,"message":"Write error )" + std::to_string(ret) + "\"}";
    return mbedtls_wrapper;
  }
  if (_body) {
    ret = mbedtls_wrapper->write(
        reinterpret_cast<const unsigned char *>(_body->c_str()), _body->size());
    if (ret < 0) {
      response->statusCode = SleepyDiscord::GENERAL_ERROR;
      response->text = R"({"code":431,"message":"Write error )" +
                       std::to_string(ret) + "\"}";
      return mbedtls_wrapper;
    }
  }

  Logger::write("Reading response\n");
  int buf_size = 1024;
  unsigned char buf[buf_size];
  int read_len;
  char last_chars[sizeof(HTTP_CHUNK_TERMINATION)] = {0};
  bool headers_done = false;
  std::string response_buf;

  http_parser parser;
  http_parser_settings parser_settings;
  bool connection_upgrade = false;

  while ((read_len = mbedtls_wrapper->read(buf, buf_size)) >= 0) {
    if (read_len > 0) {
      size_t copy_size = std::min(sizeof(HTTP_CHUNK_TERMINATION),
                                  static_cast<size_t>(read_len));
      std::memcpy(last_chars + sizeof(HTTP_CHUNK_TERMINATION) - copy_size,
                  buf + read_len - copy_size, copy_size);

      if (std::memcmp(last_chars, HTTP_CHUNK_TERMINATION, sizeof(last_chars)) ==
          0) {
        break;
      }
    }

    response_buf.append(buf, buf + read_len);
    if (!headers_done) {
      size_t index = response_buf.find("\r\n\r\n");
      if (index != std::string::npos) {
        std::string header(response_buf.c_str(), response_buf.c_str() + index);

        struct Data {
          std::string parsed_header_key;
          SleepyDiscord::Response *response;
        };
        Data data = Data{.response = response};

        http_parser_init(&parser, HTTP_RESPONSE);
        http_parser_settings_init(&parser_settings);

        parser.data = &data;

        parser_settings.on_header_field = [](http_parser *parser,
                                             const char *at, size_t length) {
          std::string key(at, at + length);
          auto data = reinterpret_cast<Data *>(parser->data);
          data->parsed_header_key = std::move(key);
          return 0;
        };

        parser_settings.on_header_value = [](http_parser *parser,
                                             const char *at, size_t length) {
          std::string value(at, at + length);
          auto data = reinterpret_cast<Data *>(parser->data);
          data->response->header[data->parsed_header_key] = std::move(value);
          return 0;
        };

        http_parser_execute(&parser, &parser_settings, header.c_str(),
                            header.size());
        response->statusCode = parser.status_code;

        headers_done = true;

        if (response->header["Connection"] == "upgrade") {
          connection_upgrade = true;
          break;
        }
      }
    }
  }

  if (!connection_upgrade) {
    http_parser_init(&parser, HTTP_RESPONSE);
    http_parser_settings_init(&parser_settings);

    parser.data = response;

    parser_settings.on_body = [](http_parser *parser, const char *at,
                                 size_t length) {
      std::string body(at, at + length);
      auto response = reinterpret_cast<SleepyDiscord::Response *>(parser->data);
      response->text = std::move(body);
      return 0;
    };

    http_parser_execute(&parser, &parser_settings, response_buf.c_str(),
                        response_buf.size());

    auto it = response->header.find("content-encoding");
    if (it != response->header.end() && it->second == "gzip") {
      Logger::write("Decompressing body\n");

      std::stringstream ss;
      ss << response->text;
      response->text.clear();
      response->text.shrink_to_fit();

      std::string decompressed;
      ZlibWrapper zlib_wrapper;
      zlib_wrapper.set_stream(&ss);
      while ((read_len = zlib_wrapper.read(reinterpret_cast<char *>(buf),
                                           sizeof(buf))) > 0) {
        decompressed.insert(decompressed.end(), buf, buf + read_len);
      }

      response->text = std::move(decompressed);
    }
    response->text.shrink_to_fit();
  }

  Logger::write("Session done %s\n", _url.c_str());

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
