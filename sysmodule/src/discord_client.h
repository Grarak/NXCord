#pragma once
#include <sleepy_discord/sleepy_discord.h>

class DiscordClient : public SleepyDiscord::BaseDiscordClient {
 private:
  std::string _token;

  bool connect(const std::string &, GenericMessageReceiver *,
               SleepyDiscord::WebsocketConnection &) override;
  void send(std::string, SleepyDiscord::WebsocketConnection &) override;
  void disconnect(unsigned int, const std::string,
                  SleepyDiscord::WebsocketConnection &) override;
  void onError(SleepyDiscord::ErrorCode, const std::string) override;
  SleepyDiscord::Timer schedule(SleepyDiscord::TimedTask code,
                                const time_t milliseconds) override;

 public:
  DiscordClient(const std::string &token);
  ~DiscordClient();

  void tick();
};
