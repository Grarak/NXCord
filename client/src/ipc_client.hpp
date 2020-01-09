#pragma once

#include <switch.h>

#include <common/ipc_structures.hpp>
#include <common/nxcord_com_interface.hpp>
#include <string>

class IPCClient : public NXCordComInterface {
 private:
  IPC_COMMAND_ENUM

  Service _service;

 public:
  IPCClient();
  ~IPCClient() override;

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
