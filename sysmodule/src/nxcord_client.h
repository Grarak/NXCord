#pragma once
#include <switch.h>

#include "discord_client.h"

class AudioReceiver;
class VoiceEventHandler;
class NXCordClient : public DiscordClient {
 private:
  void onReady(SleepyDiscord::Ready readyData) override;
  void onMessage(SleepyDiscord::Message message) override;
  void onHeartbeat() override { printf("Heartbeat sent\n"); }
  void onHeartbeatAck() override { printf("Heartbeat acknowledged\n"); }

  friend AudioReceiver;
  friend VoiceEventHandler;

 public:
  NXCordClient(const std::string& token);
  ~NXCordClient() override;
};
