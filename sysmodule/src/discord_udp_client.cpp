#include "discord_udp_client.h"

SleepyDiscord::CustomInitUDPClient SleepyDiscord::CustomUDPClient::init =
    []() -> SleepyDiscord::GenericUDPClient * { return new DiscordUDPClient; };

bool DiscordUDPClient::connect(const std::string &to, const uint16_t port) {
  if ((_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Socket creation failed");
    return false;
  }

  printf("UDP Conneting to %s:%d %d\n", to.c_str(), port, _fd);

  memset(&_servaddr, 0, sizeof(_servaddr));
  _servaddr.sin_family = AF_INET;
  _servaddr.sin_port = htons(port);
  _servaddr.sin_addr.s_addr = inet_addr(to.c_str());

  return true;
}

void DiscordUDPClient::send(const uint8_t *buffer, size_t buffer_length,
                            SendHandler handler) {
  sendto(_fd, reinterpret_cast<const char *>(buffer), buffer_length, 0,
         reinterpret_cast<const sockaddr *>(&_servaddr), sizeof(_servaddr));
  handler();
}

void DiscordUDPClient::receive(ReceiveHandler handler) {
  size_t buf_size = 1024;
  uint8_t buf[buf_size];
  socklen_t len;
  int read = recvfrom(
      _fd, buf, buf_size,
      _first_read ? MSG_WAITALL
                  : MSG_DONTWAIT,  // First read is usually receiving the ip
                                   // address, block here, otherwise we miss it
      reinterpret_cast<sockaddr *>(&_servaddr), &len);
  _first_read = false;
  handler(std::vector<uint8_t>(buf, buf + read));
}

DiscordUDPClient::~DiscordUDPClient() {
  if (_fd >= 0) {
    printf("Closing connection to %d\n", _fd);
    close(_fd);
  }
}
