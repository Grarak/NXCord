#include <switch.h>

#include "base64.h"
#include "discord_client.h"
#include "discord_session.h"

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

  start(_token);
}

DiscordClient::~DiscordClient() {}

ssize_t recv_callback(wslay_event_context_ptr ctx, uint8_t *buf, size_t len,
                      int flags, void *user_data) {
  return 0;
}

ssize_t send_callback(wslay_event_context_ptr ctx, const uint8_t *data,
                      size_t len, int flags, void *user_data) {
  return 0;
}

int genmask_callback(wslay_event_context_ptr ctx, uint8_t *buf, size_t len,
                     void *user_data) {
  return 0;
}

void on_msg_recv_callback(wslay_event_context_ptr ctx,
                          const struct wslay_event_on_msg_recv_arg *arg,
                          void *user_data) {}

bool DiscordClient::connect(const std::string &url) {
  if (!_wslay_event_context) {
    wslay_event_context_client_init(&_wslay_event_context,
                                    &_wslay_event_callbacks, this);
  }

  unsigned char random_bytes[16];

  printf("Generating random key\n");
  if (R_FAILED(csrngInitialize())) {
    printf("Failed to initialize csrng\n");
    return false;
  }

  if (R_FAILED(csrngGetRandomBytes(random_bytes, 16))) {
    printf("Failed to generate random bytes\n");
    csrngExit();
    return false;
  }

  std::string random_key = base64_encode(random_bytes, 16);

  DiscordSession session;
  session.setUrl(url);

  std::vector<SleepyDiscord::HeaderPair> header = {
      {"Upgrade", "websocket"},
      {"Connection", "Upgrade"},
      {"Sec-WebSocket-Key", random_key},
      {"Sec-WebSocket-Version", "13"}};
  session.setHeader(header);

  printf("Request conenction to websocket\n");
  SleepyDiscord::Response response;
  _mbedtls_wrapper = session.request(SleepyDiscord::Get, &response);

  if (response.statusCode !=
      SleepyDiscord::SWITCHING_PROTOCOLS) {  // error check
    printf("Websocket connection Error: %s\n", response.text.c_str());
    return false;
  }

  printf("Connection to websocket established\n");
  return true;
}

void DiscordClient::tick() {}
