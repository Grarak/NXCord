#pragma once
#include <sleepy_discord/sleepy_discord.h>

class DiscordClient : public SleepyDiscord::BaseDiscordClient {
 private:
  std::string _token;

  bool connect(const std::string&) override;

 public:
  DiscordClient(const std::string& token);
  ~DiscordClient();

  void establishConnection();
};
