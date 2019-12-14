#pragma once
#include <sleepy_discord/sleepy_discord.h>
#include <wslay/wslay.h>

#include "mbedtls_wrapper.h"

class DiscordClient : public SleepyDiscord::BaseDiscordClient {
 private:
  std::string _token;
  wslay_event_context_ptr _wslay_event_context = nullptr;
  wslay_event_callbacks _wslay_event_callbacks;

  std::shared_ptr<MBedTLSWrapper> _mbedtls_wrapper;
  bool _connected = false;

  time_t _previous_time = 0;
  std::vector<std::pair<std::function<void()>, time_t>> _scheduled_functions;

  bool connect(const std::string &) override;
  void send(std::string) override;
  void disconnect(unsigned int, const std::string) override;
  void sleep(const unsigned int) override;
  void onError(SleepyDiscord::ErrorCode, const std::string) override;
  SleepyDiscord::Timer schedule(std::function<void()> code,
                                const time_t milliseconds) override;

  bool pollSocket(int events);

 public:
  DiscordClient(const std::string &token);
  ~DiscordClient();

  void tick();

  friend ssize_t recv_callback(wslay_event_context_ptr ctx, uint8_t *buf,
                               size_t len, int flags, void *user_data);
  friend ssize_t send_callback(wslay_event_context_ptr ctx, const uint8_t *data,
                               size_t len, int flags, void *user_data);
  friend int genmask_callback(wslay_event_context_ptr ctx, uint8_t *buf,
                              size_t len, void *user_data);
  friend void on_msg_recv_callback(
      wslay_event_context_ptr ctx,
      const struct wslay_event_on_msg_recv_arg *arg, void *user_data);
};
