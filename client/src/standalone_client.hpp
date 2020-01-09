#pragma once
#ifdef STANDALONE

#include <switch.h>

#include <common/loop_thread.hpp>
#include <common/nxcord_com_interface.hpp>
#include <mutex>
#include <nxcord/nxcord_client.hpp>

class StandaloneClient : public NXCordComInterface {
 private:
  NXCordClient *_nxcord_client;
  std::mutex _nxcord_client_mutex;
  LoopThread<StandaloneClient *> _standalone_client_thread;

  friend void standalone_client_thread(StandaloneClient *client);

 public:
  StandaloneClient();
  ~StandaloneClient() override;

  bool isConnected() override;
  IPCStruct::LoginResult attemptLogin(const IPCStruct::Login &login) override;
  bool submit2faTicket(const std::string &code) override;
  bool tokenAvailable() override;
  void startConnection() override;
  void stopConnection() override;
  std::vector<IPCStruct::DiscordServer> getServers() override;
  std::vector<IPCStruct::DiscordChannel> getChannels(int64_t serverId) override;
  void joinVoiceChannel(int64_t serverId, int64_t channelId) override;
  void disconnectVoiceChannel() override;
  bool isConnectedVoiceChannel() override;
  void logout() override;
};

#endif
