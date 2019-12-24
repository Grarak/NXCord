#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <switch.h>
#include <sys/socket.h>

#include "discord_websocket.h"

ssize_t recv_callback(wslay_event_context_ptr ctx, uint8_t *buf, size_t len,
                      int flags, void *user_data) {
  DiscordWebsocket *discord_websocket =
      static_cast<DiscordWebsocket *>(user_data);

  int ret = discord_websocket->_mbedtls_wrapper->read(buf, len);
  if (ret == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      errno = EWOULDBLOCK;
      wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
    } else {
      printf("Read fail: %d\n", ret);
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
                      size_t len, int flags, void *user_data) {
  DiscordWebsocket *discord_websocket =
      static_cast<DiscordWebsocket *>(user_data);

  int ret = discord_websocket->_mbedtls_wrapper->write(data, len);
  if (ret == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      errno = EWOULDBLOCK;
      wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
    } else {
      printf("Write fail: %d\n", ret);
      wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
    }
    return -1;
  }
  return ret;
}

int genmask_callback(wslay_event_context_ptr ctx, uint8_t *buf, size_t len,
                     void *user_data) {
  if (R_FAILED(csrngGetRandomBytes(buf, len))) {
    printf("Failed to generate random bytes\n");
    return -1;
  }
  return 0;
}

void on_msg_recv_callback(wslay_event_context_ptr ctx,
                          const wslay_event_on_msg_recv_arg *arg,
                          void *user_data) {
  DiscordWebsocket *discord_websocket =
      static_cast<DiscordWebsocket *>(user_data);
  const char *msg = reinterpret_cast<const char *>(arg->msg);
  printf("Receive %lu %s\n", arg->msg_length,
         std::string(msg, msg + arg->msg_length).c_str());
  discord_websocket->_message_processor->processMessage(
      std::string(msg, msg + arg->msg_length));
}

DiscordWebsocket::DiscordWebsocket(
    SleepyDiscord::GenericMessageReceiver *message_processor,
    std::unique_ptr<MBedTLSWrapper> &mbedtls_wrapper)
    : _message_processor(message_processor),
      _mbedtls_wrapper(std::move(mbedtls_wrapper)) {
  // Make fd non blocking
  int status = fcntl(_mbedtls_wrapper->get_fd(), F_GETFL);
  fcntl(_mbedtls_wrapper->get_fd(), F_SETFL, status | O_NONBLOCK);

  int val = 1;
  if (setsockopt(_mbedtls_wrapper->get_fd(), IPPROTO_TCP, TCP_NODELAY, &val,
                 (socklen_t)sizeof(val)) == -1) {
    printf("Failed setsockopt: TCP_NODELAY");
  }

  _wslay_event_callbacks = {
      recv_callback, send_callback, genmask_callback,     NULL,
      NULL,          NULL,          on_msg_recv_callback,
  };

  wslay_event_context_client_init(&_wslay_event_context,
                                  &_wslay_event_callbacks, this);
}

DiscordWebsocket::~DiscordWebsocket() { disconnect(1000, ""); }

int DiscordWebsocket::queue_message(const std::string &message) {
  wslay_event_msg msg = {1, reinterpret_cast<const uint8_t *>(message.c_str()),
                         message.size()};
  printf("Send %s\n", message.c_str());
  return wslay_event_queue_msg(_wslay_event_context, &msg);
}

void DiscordWebsocket::disconnect(unsigned int code, const std::string reason) {
  if (_wslay_event_context) {
    wslay_event_queue_close(_wslay_event_context, code,
                            reinterpret_cast<const uint8_t *>(reason.c_str()),
                            reason.length());
    wslay_event_send(_wslay_event_context);
    wslay_event_context_free(_wslay_event_context);
    _wslay_event_context = nullptr;
  }
}

bool DiscordWebsocket::pollSocket(int events) {
  pollfd pol;
  pol.fd = _mbedtls_wrapper->get_fd();
  pol.events = events;

  if (poll(&pol, 1, 0) == 1) {
    return pol.revents & events;
  }
  return false;
}

bool DiscordWebsocket::tick() {
  int ret;
  if (wslay_event_want_read(_wslay_event_context) && pollSocket(POLLIN)) {
    if ((ret = wslay_event_recv(_wslay_event_context)) != 0) {
      printf("Wslay read failed %d\n", ret);
      return false;
    }
  }

  if (wslay_event_want_write(_wslay_event_context) && pollSocket(POLLOUT)) {
    if ((ret = wslay_event_send(_wslay_event_context)) != 0) {
      printf("Wslay write failed %d\n", ret);
      return false;
    }
  }
  return true;
}
