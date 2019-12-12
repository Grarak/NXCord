#pragma once
#include <sleepy_discord/sleepy_discord.h>
#include <wslay/wslay.h>

#include "mbedtls_wrapper.h"

class DiscordClient : public SleepyDiscord::BaseDiscordClient {
 private:
  std::string _token;
  wslay_event_context_ptr _wslay_event_context;
  wslay_event_callbacks _wslay_event_callbacks;

  std::shared_ptr<MBedTLSWrapper> _mbedtls_wrapper;

  bool connect(const std::string&) override;

 public:
  DiscordClient(const std::string& token);
  ~DiscordClient();

  void tick();
};
