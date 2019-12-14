#pragma once
#include <sleepy_discord/sleepy_discord.h>
#include <wslay/wslay.h>

#include "mbedtls_wrapper.h"

class DiscordClient : public SleepyDiscord::BaseDiscordClient {
 private:
  std::string _token;
  wslay_event_context_ptr _wslay_event_context = nullptr;
  wslay_event_callbacks _wslay_event_callbacks;

  std::unique_ptr<MBedTLSWrapper> _mbedtls_wrapper;
  bool _connected = false;

  struct ScheduledFunction {
    std::function<void()> function;
    time_t scheduled_time;
    bool enabled;
  };

  size_t _schedule_counter = 0;
  time_t _previous_time = 0;
  std::map<size_t, ScheduledFunction> _scheduled_functions;

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
