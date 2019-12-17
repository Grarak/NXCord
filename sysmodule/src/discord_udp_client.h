#pragma once
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sleepy_discord/custom_udp_client.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>

class DiscordUDPClient : public SleepyDiscord::GenericUDPClient {
 private:
  int _fd = -1;
  sockaddr_in _servaddr;
  bool _first_read = true;

 public:
  ~DiscordUDPClient();

  bool connect(const std::string& to, const uint16_t port) override;
  void send(
      const uint8_t* buffer, size_t bufferLength,
      SendHandler handler = []() {}) override;
  void receive(ReceiveHandler handler) override;
};
