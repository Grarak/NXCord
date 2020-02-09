#include <poll.h>
#include <switch.h>

#include <common/logger.hpp>
#include <common/utils.hpp>
#include <nxcord/discord_udp_client.hpp>

SleepyDiscord::CustomInitUDPClient SleepyDiscord::CustomUDPClient::init =
    []() -> SleepyDiscord::GenericUDPClient * { return new DiscordUDPClient; };

void receive_thread(DiscordUDPClient *udp_client) {
  uint8_t buf[1920];
  bool first_read = true;

  while (udp_client->_receive_thread.isActive()) {
    socklen_t len = sizeof(sockaddr_in);
    int read =
        recvfrom(udp_client->_fd, buf, sizeof(buf), MSG_WAITALL,
                 reinterpret_cast<sockaddr *>(&udp_client->_servaddr), &len);
    if (read > 0) {
      if (first_read) {
        Logger::write("UDP first read %d\n", read);
        first_read = false;
      }
      SleepyDiscord::GenericUDPClient::ReceiveHandler handler;
      {
        std::scoped_lock lock(udp_client->_receiver_func_mutex);
        handler = udp_client->receive_handler;
      }
      std::vector<uint8_t> ret(buf, buf + read);
      if (handler) {
        handler(ret);
      }
    } else if (errno != EAGAIN && errno != EWOULDBLOCK &&
               udp_client->_receive_thread.isActive()) {
      svcSleepThread(1e+9);
    }
  }
}

DiscordUDPClient::DiscordUDPClient()
    : _receive_thread(this, receive_thread, 0x4000, false) {}

DiscordUDPClient::~DiscordUDPClient() { disconnect(); }

bool DiscordUDPClient::connect(const std::string &to, const uint16_t port) {
  disconnect();

  if ((_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    Logger::write("UDP Socket creation failed");
    return false;
  }

  timeval timeout{};
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;

  setsockopt(_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

  Logger::write("UDP Connecting to %s:%d %d\n", to.c_str(), port, _fd);

  memset(&_servaddr, 0, sizeof(_servaddr));
  _servaddr.sin_family = AF_INET;
  _servaddr.sin_port = htons(port);
  _servaddr.sin_addr.s_addr = inet_addr(to.c_str());

  _receive_thread.start();

  return true;
}

void DiscordUDPClient::disconnect() {
  if (_fd >= 0) {
    int fd = _fd;
    _receive_thread.stop([fd]() {
      Logger::write("UDP Closing connection to %d\n", fd);
      shutdown(fd, SHUT_RDWR);
      close(fd);
    });
    _fd = -1;
  }
}

void DiscordUDPClient::send(const uint8_t *buffer, size_t buffer_length,
                            SendHandler handler) {
  if (_fd >= 0) {
    sendto(_fd, reinterpret_cast<const char *>(buffer), buffer_length,
           MSG_DONTWAIT, reinterpret_cast<const sockaddr *>(&_servaddr),
           sizeof(_servaddr));
  }
  handler();
}
