#pragma once
#include "discord_client.h"

class NXCordClient : public DiscordClient {
 private:
  void onReady(SleepyDiscord::Ready readyData) override;
  void onMessage(SleepyDiscord::Message message) override;

 public:
  NXCordClient(const std::string& token);
};
