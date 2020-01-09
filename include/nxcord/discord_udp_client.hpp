#pragma once
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sleepy_discord/sleepy_discord.h>
#include <switch.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <common/loop_thread.hpp>
#include <cstring>
#include <mutex>

class DiscordUDPClient : public SleepyDiscord::GenericUDPClient {
 private:
  int _fd = -1;
  sockaddr_in _servaddr;

  LoopThread<DiscordUDPClient*> _receive_thread;

  std::mutex _receiver_func_mutex;
  friend void receive_thread(DiscordUDPClient* udp_client);

  std::atomic<Barrier*> _sync_barrier;
  std::vector<uint8_t> _sync_buf;

  void disconnect();

 public:
  DiscordUDPClient();
  ~DiscordUDPClient() override;

  bool connect(const std::string& to, const uint16_t port) override;
  void send(
      const uint8_t* buffer, size_t bufferLength,
      SendHandler handler = []() {}) override;
  void setReceiveHandler(ReceiveHandler handler) override;
  void unsetReceiveHandler() override;
  std::vector<uint8_t> waitForReceive() override;
};
