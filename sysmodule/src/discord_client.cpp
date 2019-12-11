#include "discord_client.h"

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

bool DiscordClient::connect(const std::string &url) { return false; }

void DiscordClient::tick() {}
