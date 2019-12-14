#include <fcntl.h>
#include <poll.h>
#include <switch.h>
#include <switch/crypto/sha1.h>

#include "base64.h"
#include "discord_client.h"
#include "discord_session.h"

#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

ssize_t recv_callback(wslay_event_context_ptr ctx, uint8_t *buf, size_t len,
                      int flags, void *user_data);
ssize_t send_callback(wslay_event_context_ptr ctx, const uint8_t *data,
                      size_t len, int flags, void *user_data);
int genmask_callback(wslay_event_context_ptr ctx, uint8_t *buf, size_t len,
                     void *user_data);
void on_msg_recv_callback(wslay_event_context_ptr ctx,
                          const struct wslay_event_on_msg_recv_arg *arg,
                          void *user_data);

DiscordClient::DiscordClient(const std::string &token) : _token(token) {
  _wslay_event_callbacks = {
      recv_callback, send_callback, genmask_callback,     NULL,
      NULL,          NULL,          on_msg_recv_callback,
  };

  if (R_FAILED(csrngInitialize())) {
    printf("Failed to initialize csrng\n");
    return;
  }

  start(_token, 1);
}

DiscordClient::~DiscordClient() {
  disconnect(0, "");
  csrngExit();
}

ssize_t recv_callback(wslay_event_context_ptr ctx, uint8_t *buf, size_t len,
                      int flags, void *user_data) {
  DiscordClient *discord_client = static_cast<DiscordClient *>(user_data);

  int ret = discord_client->_mbedtls_wrapper->read(buf, len);
  if (ret <= 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      errno = EWOULDBLOCK;
      wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
    } else {
      wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
    }

    printf("Read fail: %d\n", ret);
    return -1;
  }
  return ret;
}

ssize_t send_callback(wslay_event_context_ptr ctx, const uint8_t *data,
                      size_t len, int flags, void *user_data) {
  DiscordClient *discord_client = static_cast<DiscordClient *>(user_data);

  int ret = discord_client->_mbedtls_wrapper->write(data, len);
  if (ret == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      errno = EWOULDBLOCK;
      wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
    } else {
      wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
    }

    printf("Write fail: %d\n", ret);
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
                          const struct wslay_event_on_msg_recv_arg *arg,
                          void *user_data) {
  DiscordClient *discord_client = static_cast<DiscordClient *>(user_data);
  discord_client->processMessage(
      std::string(reinterpret_cast<const char *>(arg->msg)));
}

std::string create_acceptkey(const std::string &clientkey) {
  std::string s = clientkey + WS_GUID;
  unsigned char sha1[SHA1_HASH_SIZE];
  memset(sha1, 0, SHA1_HASH_SIZE);
  sha1CalculateHash(sha1, s.c_str(), s.size());
  return base64_encode(sha1, SHA1_HASH_SIZE);
}

bool DiscordClient::connect(const std::string &url) {
  if (!_wslay_event_context) {
    wslay_event_context_client_init(&_wslay_event_context,
                                    &_wslay_event_callbacks, this);
  }

  unsigned char random_bytes[16];

  printf("Generating random key\n");
  if (R_FAILED(csrngGetRandomBytes(random_bytes, 16))) {
    printf("Failed to generate random bytes\n");
    return false;
  }

  std::string client_key = base64_encode(random_bytes, 16);

  DiscordSession session;
  session.setUrl(url);

  std::vector<SleepyDiscord::HeaderPair> header = {
      {"Upgrade", "websocket"},
      {"Connection", "Upgrade"},
      {"Sec-WebSocket-Key", client_key},
      {"Sec-WebSocket-Version", "13"}};
  session.setHeader(header);

  printf("Request connection to websocket\n");
  SleepyDiscord::Response response;
  _mbedtls_wrapper = std::move(session.request(SleepyDiscord::Get, &response));

  if (response.statusCode !=
      SleepyDiscord::SWITCHING_PROTOCOLS) {  // error check
    printf("Websocket connection Error: %s\n", response.text.c_str());
    return false;
  }

  auto it = response.header.find("sec-websocket-accept");
  if (it == response.header.end()) {
    printf("Couldn't parse sec-websocket-accept\n");
    return false;
  }

  std::string accept_key = create_acceptkey(client_key);
  if (it->second != accept_key) {
    printf("Accept key is invalid\n");
    return false;
  }

  // Make fd non blocking
  int status = fcntl(_mbedtls_wrapper->get_fd(), F_GETFL);
  fcntl(_mbedtls_wrapper->get_fd(), F_SETFL, status | O_NONBLOCK);

  printf("Connection to websocket established\n");
  _connected = true;
  return true;
}

void DiscordClient::send(std::string message) {
  struct wslay_event_msg msg = {
      1, reinterpret_cast<const uint8_t *>(message.c_str()), message.length()};
  const int returned = wslay_event_queue_msg(_wslay_event_context, &msg);
  if (returned != 0) {  // error
    printf("Send error: ");
    switch (returned) {
      case WSLAY_ERR_NO_MORE_MSG:
        printf("Could not queue given message\n");
        break;
      case WSLAY_ERR_INVALID_ARGUMENT:
        printf("The given message is invalid\n");
        break;
      case WSLAY_ERR_NOMEM:
        printf("Out of memory\n");
        break;
      default:
        printf("unknown\n");
        break;
    }
  }
}

void DiscordClient::disconnect(unsigned int code, const std::string reason) {
  printf("Disconnecting client %s\n", reason.c_str());
  if (_wslay_event_context) {
    wslay_event_queue_close(_wslay_event_context, code,
                            reinterpret_cast<const uint8_t *>(reason.c_str()),
                            reason.length());
    wslay_event_send(_wslay_event_context);
    wslay_event_context_free(_wslay_event_context);
    _wslay_event_context = nullptr;
  }
}

void DiscordClient::sleep(const unsigned int milliseconds) {
  svcSleepThread(1e6 * milliseconds);
}

void DiscordClient::onError(SleepyDiscord::ErrorCode errorCode,
                            const std::string errorMessage) {
  printf("Error %i: %s\n", errorCode, errorMessage.c_str());
}

SleepyDiscord::Timer DiscordClient::schedule(std::function<void()> code,
                                             const time_t milliseconds) {
  size_t id = _schedule_counter++;
  auto instance = this;
  ScheduledFunction scheduled_function = {std::move(code), milliseconds, true};
  _scheduled_functions[_schedule_counter] = scheduled_function;
  return SleepyDiscord::Timer([instance, id]() {
    auto it = instance->_scheduled_functions.find(id);
    if (it != instance->_scheduled_functions.end()) {
      it->second.enabled = false;
    }
  });
}

bool DiscordClient::pollSocket(int events) {
  pollfd pol;
  pol.fd = _mbedtls_wrapper->get_fd();
  pol.events = events;

  if (poll(&pol, 1, 0) == 1) {
    return pol.revents & events;
  }
  return false;
}

void DiscordClient::tick() {
  if (!_connected) {
    return;
  }

  if (_previous_time == 0) {
    _previous_time = getEpochTimeMillisecond();
  } else {
    time_t current_time = getEpochTimeMillisecond();
    time_t diff = current_time - _previous_time;

    for (auto &pair : _scheduled_functions) {
      if (pair.second.enabled) {
        pair.second.scheduled_time -= diff;
        if (pair.second.scheduled_time <= 0) {
          pair.second.function();
          pair.second.enabled = false;
        }
      }
    }

    // Remove disabled functions
    auto it = _scheduled_functions.begin();
    while (it != _scheduled_functions.end()) {
      if (!it->second.enabled) {
        it = _scheduled_functions.erase(it);
      } else {
        ++it;
      }
    }
    _previous_time = current_time;
  }

  if (pollSocket(POLLIN) && wslay_event_want_read(_wslay_event_context)) {
    printf("Wslay Receive\n");
    if (wslay_event_recv(_wslay_event_context)) {
      return;
    }
  }

  if (pollSocket(POLLOUT) && wslay_event_want_write(_wslay_event_context)) {
    printf("Wslay Send\n");
    wslay_event_send(_wslay_event_context);
  }
}
