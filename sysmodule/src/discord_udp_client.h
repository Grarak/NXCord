#pragma once
#include <sleepy_discord/custom_udp_client.h>

class DiscordUDPClient : public SleepyDiscord::GenericUDPClient {
 public:
  bool connect(const std::string& to, const uint16_t port) override;
  void send(
      const uint8_t* buffer, size_t bufferLength,
      SendHandler handler = []() {}) override;
  void receive(ReceiveHandler handler) override;
};
