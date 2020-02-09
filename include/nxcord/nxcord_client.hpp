#pragma once

#include <common/ipc_structures.hpp>
#include <common/logger.hpp>
#include <common/nxcord_com_interface.hpp>
#include <mutex>
#include <queue>

#include "discord_client.hpp"
#include "nxcord_settings.hpp"

class AudioReceiver;

class VoiceEventHandler;

class NXCordClient : public DiscordClient {
 private:
  struct LocalVoiceContext {
    SleepyDiscord::Snowflake<SleepyDiscord::Server> serverID;
    SleepyDiscord::Snowflake<SleepyDiscord::Channel> channelID;
    std::string sessionID;
    std::string endpoint;
    std::string token;

    LocalVoiceContext() = default;

    explicit LocalVoiceContext(const SleepyDiscord::VoiceContext &context)
        : serverID(context.getServerID()),
          channelID(context.getChannelID()),
          sessionID(context.sessionID),
          endpoint(context.endpoint),
          token(context.token) {}
  };

  bool _ready = false;
  std::unique_ptr<NXCordSettings> _settings;
  LocalVoiceContext _current_voice_context;
  SleepyDiscord::VoiceConnection *_current_voice_connection = nullptr;
  std::vector<IPCStruct::DiscordServer> _servers;
  std::map<int64_t, std::vector<IPCStruct::DiscordChannel>> _channels;

  void onReady(SleepyDiscord::Ready readyData) override;

  inline void onDisconnect() override {
    _ready = false;
    _current_voice_connection = nullptr;
  }

  void onResumed() override;

  void onServer(SleepyDiscord::Server server) override;

  void onEditServer(SleepyDiscord::Server editServer) override;

  void onDeleteServer(
      SleepyDiscord::UnavailableServer unavailableServer) override;

  void addServer(const SleepyDiscord::Server &server);

  void onChannel(SleepyDiscord::Channel channel) override;

  void onEditChannel(SleepyDiscord::Channel channel) override;

  void onDeleteChannel(SleepyDiscord::Channel channel) override;

  void fillChannels(
      const SleepyDiscord::Snowflake<SleepyDiscord::Server> &channel);

  inline std::vector<IPCStruct::DiscordChannel> *getChannelsPtr(
      const SleepyDiscord::Snowflake<SleepyDiscord::Server> &serverId) {
    auto it = _channels.find(serverId.number());
    return it == _channels.end() ? nullptr : &it->second;
  }

  inline std::vector<IPCStruct::DiscordChannel> *getChannelsPtr(
      const SleepyDiscord::Channel &channel) {
    return getChannelsPtr(channel.serverID);
  }

  inline void onHeartbeat() override { Logger::write("Heartbeat sent\n"); }

  inline void onHeartbeatAck() override {
    Logger::write("Heartbeat acknowledged\n");
  }

  friend AudioReceiver;
  friend VoiceEventHandler;

 public:
  void loadSettings(std::unique_ptr<NXCordSettings> &settings);

  void startConnection() override;

  inline bool isConnected() const { return _ready && connection != nullptr; }

  inline bool isConnecting() const { return !isConnected() && !isQuiting(); }

  inline void logout() {
    quit();
    _token = "";
    if (_settings) {
      _settings->settoken("");
    }
  }

  inline const std::vector<IPCStruct::DiscordServer> &getCachedServers() const {
    return _servers;
  }

  const std::vector<IPCStruct::DiscordChannel> &getCachedChannels(
      int64_t serverId);

  void joinVoiceChannel(int64_t serverId, int64_t channelId);

  bool isConnectedVoiceChannel();

  void disconnectVoiceChannel();
};
