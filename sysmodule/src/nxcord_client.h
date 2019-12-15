#pragma once
#include "discord_client.h"

class NXCordClient : public DiscordClient {
 private:
  void onReady(std::string* jsonMessage) override;

 public:
  NXCordClient(const std::string& token);
};
