#include <poll.h>
#include <switch.h>

#include <common/logger.hpp>
#include <common/utils.hpp>
#include <nxcord/discord_udp_client.hpp>

SleepyDiscord::CustomInitUDPClient SleepyDiscord::CustomUDPClient::init =
    []() -> SleepyDiscord::GenericUDPClient * { return new DiscordUDPClient; };

void receive_thread(DiscordUDPClient *udp_client) {
  size_t buf_size = 1920;
  uint8_t buf[buf_size];
  socklen_t len;
  bool first_read = true;

  while (udp_client->_receive_thread.isActive()) {
    int read =
        recvfrom(udp_client->_fd, buf, sizeof(buf), MSG_WAITALL,
                 reinterpret_cast<sockaddr *>(&udp_client->_servaddr), &len);
    if (first_read) {
      Logger::write("UDP first read %d\n", read);
      first_read = false;
    }
    if (read > 0) {
      udp_client->_receiver_func_mutex.lock();
      SleepyDiscord::GenericUDPClient::ReceiveHandler handler =
          udp_client->receive_handler;
      udp_client->_receiver_func_mutex.unlock();

      std::vector<uint8_t> ret(buf, buf + read);
      if (handler) {
        handler(ret);
      }
    } else {
      svcSleepThread(
          2e+7);  // Always sleep for some time. Otherwise it might crash.
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
  sendto(_fd, reinterpret_cast<const char *>(buffer), buffer_length, 0,
         reinterpret_cast<const sockaddr *>(&_servaddr), sizeof(_servaddr));
  handler();
}
