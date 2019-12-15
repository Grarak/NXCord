#pragma once
#include <sleepy_discord/message_receiver.h>
#include <sleepy_discord/websocket.h>
#include <wslay/wslay.h>

#include <memory>

#include "mbedtls_wrapper.h"

class DiscordWebsocket : public SleepyDiscord::GenericWebsocketConnection {
 private:
  SleepyDiscord::GenericMessageReceiver *_message_processor;
  std::unique_ptr<MBedTLSWrapper> _mbedtls_wrapper;

  wslay_event_context_ptr _wslay_event_context = nullptr;
  wslay_event_callbacks _wslay_event_callbacks;

  bool pollSocket(int events);

  friend ssize_t recv_callback(wslay_event_context_ptr ctx, uint8_t *buf,
                               size_t len, int flags, void *user_data);
  friend ssize_t send_callback(wslay_event_context_ptr ctx, const uint8_t *data,
                               size_t len, int flags, void *user_data);
  friend int genmask_callback(wslay_event_context_ptr ctx, uint8_t *buf,
                              size_t len, void *user_data);
  friend void on_msg_recv_callback(
      wslay_event_context_ptr ctx,
      const struct wslay_event_on_msg_recv_arg *arg, void *user_data);

 public:
  DiscordWebsocket(SleepyDiscord::GenericMessageReceiver *message_processor,
                   std::unique_ptr<MBedTLSWrapper> &mbedtls_wrapper);
  ~DiscordWebsocket();

  int queue_message(const std::string &message);
  void disconnect(unsigned int, const std::string);
  void tick();
};
