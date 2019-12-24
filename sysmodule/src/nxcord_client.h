#pragma once
#include <switch.h>

#include "discord_client.h"
#include "logger.h"

constexpr size_t opus_framesize_bytes =
    SleepyDiscord::AudioTransmissionDetails::proposedLength() *
    sizeof(SleepyDiscord::AudioSample);

class AudioReceiver;
class VoiceEventHandler;
class NXCordClient : public DiscordClient {
 private:
  SleepyDiscord::VoiceContext* _current_voice_context = nullptr;

  void onReady(SleepyDiscord::Ready readyData) override;
  void onResumed() override;
  void onMessage(SleepyDiscord::Message message) override;
  void onHeartbeat() override { Logger::write("Heartbeat sent\n"); }
  void onHeartbeatAck() override { Logger::write("Heartbeat acknowledged\n"); }

  friend AudioReceiver;
  friend VoiceEventHandler;

 public:
  NXCordClient(const std::string& token);
};
