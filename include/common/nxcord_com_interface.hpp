#pragma once

#include <common/ipc_structures.hpp>
#include <string>
#include <vector>

class NXCordComInterface {
 public:
  virtual ~NXCordComInterface() = default;

  virtual bool isConnected() = 0;
  virtual IPCStruct::LoginResult attemptLogin(
      const IPCStruct::Login& login) = 0;
  virtual bool submit2faTicket(const std::string& code) = 0;
  virtual bool tokenAvailable() = 0;
  virtual void startConnection() = 0;
  virtual void stopConnection() = 0;
  virtual std::vector<IPCStruct::DiscordServer> getServers() = 0;
  virtual std::vector<IPCStruct::DiscordChannel> getChannels(
      int64_t serverId) = 0;
  virtual void joinVoiceChannel(int64_t serverId, int64_t channelId) = 0;
  virtual void disconnectVoiceChannel() = 0;
  virtual bool isConnectedVoiceChannel() = 0;
  virtual void logout() = 0;

  static IPCStruct::Login create_login(const std::string& email,
                                       const std::string& password) {
    IPCStruct::Login login;
    std::strncpy(login.email, email.c_str(), sizeof(login.email) - 1);
    std::strncpy(login.password, password.c_str(), sizeof(login.password) - 1);
    return login;
  }

  static IPCStruct::DiscordServer create_discord_server(const std::string& name,
                                                        int64_t id) {
    IPCStruct::DiscordServer discord_server;
    std::strncpy(discord_server.name, name.c_str(),
                 sizeof(discord_server.name) - 1);
    discord_server.id = id;
    return discord_server;
  }

  static IPCStruct::DiscordChannel create_discord_channel(
      const std::string& name, int64_t serverId, int64_t id,
      IPCStruct::DiscordChannelType type) {
    IPCStruct::DiscordChannel discord_channel;
    std::strncpy(discord_channel.name, name.c_str(),
                 sizeof(discord_channel.name) - 1);
    discord_channel.serverId = serverId;
    discord_channel.id = id;
    discord_channel.type = type;
    return discord_channel;
  }
};
