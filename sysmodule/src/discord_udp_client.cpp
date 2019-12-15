#include "discord_udp_client.h"

SleepyDiscord::CustomInitUDPClient SleepyDiscord::CustomUDPClient::init =
    []() -> SleepyDiscord::GenericUDPClient* { return new DiscordUDPClient; };

bool DiscordUDPClient::connect(const std::string& to, const uint16_t port) {
  return false;
}
void DiscordUDPClient::send(const uint8_t* buffer, size_t bufferLength,
                            SendHandler handler) {}
void DiscordUDPClient::receive(ReceiveHandler handler) {}
