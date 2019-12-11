#pragma once
#include <sleepy_discord/sleepy_discord.h>
#include <wslay/wslay.h>

class DiscordClient : public SleepyDiscord::BaseDiscordClient {
 private:
  std::string _token;
  wslay_event_context_ptr _wslay_event_context;
  wslay_event_callbacks _wslay_event_callbacks;

  bool connect(const std::string&) override;

 public:
  DiscordClient(const std::string& token);
  ~DiscordClient();

  void tick();
};
