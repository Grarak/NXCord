#include <fcntl.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <switch.h>

#include <common/logger.hpp>
#include <cstring>
#include <nxcord/discord_websocket.hpp>

ssize_t recv_callback(wslay_event_context_ptr ctx, uint8_t *buf, size_t len,
                      int, void *user_data) {
  auto discord_websocket = static_cast<DiscordWebsocket *>(user_data);

  int ret = discord_websocket->_mbedtls_wrapper->read(buf, len);
  if (ret == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      errno = EWOULDBLOCK;
      wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
    } else {
      Logger::write("Read fail: %d\n", ret);
      wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
    }
    return -1;
  } else if (ret == 0) {
    wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
    return -1;
  }
  return ret;
}

ssize_t send_callback(wslay_event_context_ptr ctx, const uint8_t *data,
                      size_t len, int, void *user_data) {
  auto discord_websocket = static_cast<DiscordWebsocket *>(user_data);

  int ret = discord_websocket->_mbedtls_wrapper->write(data, len);
  if (ret == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      errno = EWOULDBLOCK;
      wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
    } else {
      Logger::write("Write fail: %d\n", ret);
      wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
    }
    return -1;
  }
  return ret;
}

int genmask_callback(wslay_event_context_ptr, uint8_t *buf, size_t len,
                     void *) {
  if (R_FAILED(csrngGetRandomBytes(buf, len))) {
    Logger::write("Failed to generate random bytes\n");
    return -1;
  }
  return 0;
}

void on_msg_recv_callback(wslay_event_context_ptr,
                          const wslay_event_on_msg_recv_arg *arg,
                          void *user_data) {
  auto discord_websocket = static_cast<DiscordWebsocket *>(user_data);
  const char *msg = reinterpret_cast<const char *>(arg->msg);
  std::string str_msg;

  if (discord_websocket->_zlib_compress) {
    std::vector<char> &_zlib_buf = discord_websocket->_zlib_buf;
    _zlib_buf.insert(_zlib_buf.end(), msg, msg + arg->msg_length);
    if (arg->msg_length >= 4 &&
        std::memcmp(msg + arg->msg_length - 4, DiscordWebsocket::zlib_suffix,
                    4) == 0) {
      str_msg = discord_websocket->_zlib_wrapper.decompress(_zlib_buf.data(),
                                                            _zlib_buf.size());
      _zlib_buf.clear();
    }
  } else {
    str_msg = std::string(msg, msg + arg->msg_length);
  }

  if (!str_msg.empty()) {
    std::string log(str_msg.c_str(),
                    str_msg.c_str() + std::min(str_msg.size(),
                                               static_cast<size_t>(
                                                   500)));  // Limit log output
    Logger::write("Receive %s\n", log.c_str());
    discord_websocket->_message_processor->processMessage(str_msg);
    Logger::write("Message processed\n");
  }
}

DiscordWebsocket::DiscordWebsocket(
    SleepyDiscord::GenericMessageReceiver *message_processor,
    std::unique_ptr<MBedTLSWrapper> &mbedtls_wrapper, bool zlib_compress)
    : _message_processor(message_processor),
      _mbedtls_wrapper(std::move(mbedtls_wrapper)),
      _zlib_compress(zlib_compress) {
  // Make fd non blocking
  int flags, r;
  while ((flags = fcntl(_mbedtls_wrapper->getFd(), F_GETFL, 0)) == -1 &&
         errno == EINTR)
    ;
  while ((r = fcntl(_mbedtls_wrapper->getFd(), F_SETFL, flags | O_NONBLOCK)) ==
             -1 &&
         errno == EINTR)
    ;
  NXC_ASSERT(flags != -1 && r != -1);

  _wslay_event_callbacks = {
      recv_callback, send_callback, genmask_callback,     nullptr,
      nullptr,       nullptr,       on_msg_recv_callback,
  };

  wslay_event_context_client_init(&_wslay_event_context,
                                  &_wslay_event_callbacks, this);
}

DiscordWebsocket::~DiscordWebsocket() { disconnect(1000, ""); }

int DiscordWebsocket::queue_message(const std::string &message) {
  wslay_event_msg msg = {1, reinterpret_cast<const uint8_t *>(message.c_str()),
                         message.size()};
  Logger::write("Send %s\n", message.c_str());
  return wslay_event_queue_msg(_wslay_event_context, &msg);
}

void DiscordWebsocket::disconnect(unsigned int code,
                                  const std::string &reason) {
  if (_wslay_event_context) {
    wslay_event_queue_close(_wslay_event_context, code,
                            reinterpret_cast<const uint8_t *>(reason.c_str()),
                            reason.length());
    wslay_event_send(_wslay_event_context);
    wslay_event_context_free(_wslay_event_context);
    _wslay_event_context = nullptr;
  }
}

bool DiscordWebsocket::pollSocket(uint16_t events) {
  pollfd pol{};
  pol.fd = _mbedtls_wrapper->getFd();
  pol.events = events;

  if (poll(&pol, 1, 0) == 1) {
    return pol.revents & events;
  }
  return false;
}

void DiscordWebsocket::tick() {
  int ret;
  if (wslay_event_want_read(_wslay_event_context) && pollSocket(POLLIN)) {
    if ((ret = wslay_event_recv(_wslay_event_context)) != 0) {
      Logger::write("Wslay read failed %d\n", ret);
      _message_processor->processCloseCode(1000);
      return;
    }
  }

  if (wslay_event_want_write(_wslay_event_context) && pollSocket(POLLOUT)) {
    if ((ret = wslay_event_send(_wslay_event_context)) != 0) {
      Logger::write("Wslay write failed %d\n", ret);
      _message_processor->processCloseCode(1000);
    }
  }
}
